// Bp sprite script commands: the handlers that create and remove sprite objects from script
// data, queue position, scale, opacity, rotation, tint and tile animation, set pivots, and draw quads.

#include <decomp/static_init.h>
#include "classes/BpSpriteDrawView.h"
#define AsBpDrawObject800BC91C(p) ((BpSpriteDrawView*)(p))

extern "C" {

enum BpAnimationChannel {
    kChannelPositionX = 6,
    kChannelPositionY = 7,
    kChannelScaleX = 8,
    kChannelScaleY = 9,
    kChannelOpacity = 10,
    kChannelUnidentified11 = 11,
    kChannelTileColumns = 12,
    kChannelTileRows = 13,
    kChannelRotation = 16,
    kChannelTint = 17,
    kChannelCount = 18
};
enum {
    kBpSlotCount = 40,
    kBpSlotBytes = 0x224,
    kBpSlotStorageBytes = 0x55A0,
    kBpScaleOne = 1024,
    kBpOpacityOne = 128,
    kControlReset = 1,
    kControlEnabled = 2,
    kControlHold = 4,
    kControlHeld = 8,
    kAnimationHoldRequest = 0x400,
    kAnimationPreserveValue = 0x800,
    kBpChannelControlOffset = 0x210,
    kDrawBlendBits = 0xF0,
    kDrawFormatBits = 0xF000
};
// Command slots are separate from the renderer's 0x34-byte controller entries.
struct BpCommandSlot {
    unsigned int state;
    unsigned int fileBlock;
    char padding[0x200];
    unsigned int selectedId;
    unsigned int selectedObject;
    signed char channelControl[kChannelCount];
    char tailPadding[2];
};
struct BpFileBlock {
    unsigned int header[2];
    unsigned int dataOffset;
};
struct BpObjectParentChannel {
    char padding[0x90];
    unsigned int previousParentId, parentId;
};
struct BpPivotChannelDefaults {
    char padding[0x168];
    unsigned int pivotX;
    char paddingY[0x14];
    unsigned int pivotY;
};

// Script headers occupy two words. Payload layouts differ by command.
struct BpScalarAnimationCommand {
    unsigned int commandWord, commandSize, value, flags, time;
};
struct BpPairAnimationCommand {
    unsigned int commandWord, commandSize, x, y, flags, time;
};
// Unlike scalar/pair commands, the tile command has no separate time word:
// its timeAndControl member controls columns; rows use 0.
// Both calls pass zero as commandFlags. Do not copy scalar command flag handling.
struct BpTileAnimationCommand {
    unsigned int commandWord, commandSize, columns, rows, timeAndControl;
};
struct BpPivotCommand {
    unsigned int commandWord, commandSize, x, y;
};
struct BpAnimationBatchCommand {
    unsigned int commandWord, commandSize, values[5], flags, time;
};

// Generic command words retained for entry points with command-specific external operands.
// SelectItemAndResetControlFlags consumes operand0 as the selected object ID;
// QueueImmediateChannel11 instead borrows that SAME operand0 as its channel value.
// This shared command view therefore keeps a neutral operand0 name. Operands1..6
// are not consumed here and have no inferred meanings in this unit.
struct BF2FCArgs {
    unsigned int commandWord;
    unsigned int commandSize;
    unsigned int operand0;
    unsigned int operand1;
    unsigned int operand2;
    unsigned int operand3;
    unsigned int operand4;
    unsigned int operand5;
    unsigned int operand6;
};

extern unsigned int g_bpCommandSlots;
extern const char b2dSourceFileName[];
extern const char b2dFlagControlAllDiagnostic[];
extern const char b2dFileLabelHookNames[];
extern const char b2dAnimationChannelDiagnostic[];
unsigned int __nwa__FUl(unsigned int size);
// Five (channelId, valueIndex) pairs. valueIndex addresses values[] in the batch
// command, not a byte offset or an ordinal in BpAnimationChannel. This external
// table defines dispatch order; failure stops the batch without undoing earlier
// enqueues. It must contain valid channel IDs and indices 0..4.
struct BF0FCTable {
    unsigned int values[10];
};
extern BF0FCTable kBpAnimationBatchChannelMap;
int BpController_ResetAnimationState(void* object, int type);
int BpCtrl_AppendAnimationChannelNode(int index, void* object, int type, unsigned int flags, unsigned int a,
                                      unsigned int b);
int BpCtrl_CreateDrawObjectFlag3(int index, unsigned int value, unsigned int offset);
int BpCtrl_CreateDrawObjectFlag2(int index, unsigned int value, unsigned int a, unsigned int b, unsigned int c,
                                 unsigned int offset);
void* BpSlot_FindItemById(int index, unsigned int value);
int B2dInp_SetUserData(int index, unsigned int value);
void BpCtrl_InvokeGlobalHook();
int BpSprite_ComputeRotatedQuad(unsigned int* out, void* self, int index);
unsigned int BpCtrl_GetColoredQuadCallback();
unsigned int BpCtrl_GetTexturedQuadCallback();
int BpCtrl_CreateDrawObject(int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                            unsigned int, unsigned int);
int Color_ModulatePackedChannels(int, unsigned int);
void* BpSlot_FindItemById(int, unsigned int);

// Object-creation command schema, all fields are 32-bit script words:
// +00 command, +04 size; +08 selectedObjectId = selected object ID;
// +0C..+20 operands1..6 = six creation properties forwarded unchanged (individual
// property meanings unresolved in this wrapper); +24 fileDataOffset = file-data offset;
// +28/+2C operands8/9 are unused here; +30 initialParentId = initial parent object ID.
// Resolved payload address = fileBlock + fileBlock->dataOffset + fileDataOffset.
// Flag2/Flag3 variants below have DIFFERENT schemas: ID at args[2], data offset
// at args[6], parent at args[9]. Do not cast these to the full-creation layout.
// Six unclassified creation properties must stay in their observed order. Known
// ID/data-offset/parent field names describe their command-specific roles.
struct BE8D0Args {
    unsigned int commandWord, commandSize, selectedObjectId, operand1, operand2, operand3, operand4, operand5;
    unsigned int operand6, fileDataOffset, operand8, operand9, initialParentId;
};

// Resolve script data, create the selected draw object, and initialize both parent-ID values.
// index must already be a valid slot: selectedId is written before bounds diagnostics.
// On success synchronize current/previous parent ID; return 0 or the negative creator
// error. A diagnostic resolves payload to zero but does not cancel object creation.
int BpCmd_CreateObject(int index, BE8D0Args* args) {
    // Phase 1: select the object ID and resolve the slot-relative script data.
    unsigned int payloadAddress;
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);

    int offset = index * kBpSlotBytes;
    ((BpCommandSlot*)(g_bpCommandSlots + offset))->selectedId = args->selectedObjectId;
    payloadAddress = args->fileDataOffset;
    if (index < 0 || index >= kBpSlotCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(-1, b2dSourceFileName, 0x90,
                                                                               b2dFileLabelHookNames);
        payloadAddress = 0;
    } else {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        unsigned int block = ((BpCommandSlot*)(g_bpCommandSlots + offset))->fileBlock;
        if (block == 0) {
            ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(-2, b2dSourceFileName, 0x98,
                                                                                   b2dFileLabelHookNames);
            payloadAddress = 0;
        } else {
            payloadAddress += block + ((BpFileBlock*)block)->dataOffset;
        }
    }
    // Phase 2: create the draw object; only nonnegative results initialize its parent channel.
    if (0 == g_bpCommandSlots)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    int result = BpCtrl_CreateDrawObject(index, ((BpCommandSlot*)(g_bpCommandSlots + offset))->selectedId,
                                         args->operand1, args->operand2, args->operand3, args->operand4, args->operand5,
                                         args->operand6, payloadAddress);
    if (0 <= result) {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        void* object = BpSlot_FindItemById(index, ((BpCommandSlot*)(g_bpCommandSlots + offset))->selectedId);
        ((BpObjectParentChannel*)object)->parentId = args->initialParentId;
        ((BpObjectParentChannel*)object)->previousParentId = args->initialParentId;
    }
    return ((int)result >> 31) & (int)result;
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Transform a textured quad and submit its UV bounds, tint, opacity, and render flags.
// x/y translate the rotated quad in draw coordinates; sx/sy are scales with 1024=1.
// opacityScale and object/parent opacity use 128=1. rect is four interleaved x/y
// corners; source bounds are the image's x/y/width/height in texture-region units.
// Callback order: image, transformed corners, source corners, opacity, packed tint,
// flags. tintColor is carried through a pointer-typed ABI word; never dereference it.
// Arrays are stack views valid only for this synchronous callback; callback ownership
// or copying of image resources is external to this function.
// Typed synchronous callback contracts; packedTint is a color WORD in a pointer-
// typed register, and must never be treated as a color-buffer address.
typedef void (*TexturedQuadSubmit)(unsigned int* image, unsigned int* xyCorners, unsigned int* sourceCorners,
                                   int opacity128, unsigned int* packedTint, unsigned int flags);
typedef void (*ColoredQuadSubmit)(void* sourceColorBlock, unsigned int* xyCorners, int opacity128, int combinedFlags,
                                  unsigned int fullFormatWord);
int BpSprite_DrawTransformedQuad(void* self, int parentObject, int x, int y, int sx, int sy, int opacityScale) {
    unsigned int* drawCallback = ((unsigned int*)AsBpDrawObject800BC91C(self)->drawQuadCallback);
    int opacity = AsBpDrawObject800BC91C(self)->opacity;
    unsigned int* packedTintWord = ((unsigned int*)AsBpDrawObject800BC91C(self)->tintColor);
    if (drawCallback == 0) {
        drawCallback = (unsigned int*)BpCtrl_GetTexturedQuadCallback();
    }
    // Phase 1: compute rotation/parent placement, then apply this draw's scale and translation.
    unsigned int rect[8];
    BpSprite_ComputeRotatedQuad(rect, self, parentObject);
    rect[0] = x + (sx * (int)rect[0] / kBpScaleOne);
    rect[1] = y + (sy * (int)rect[1] / kBpScaleOne);
    rect[2] = x + (sx * (int)rect[2] / kBpScaleOne);
    rect[3] = y + (sy * (int)rect[3] / kBpScaleOne);
    rect[4] = x + (sx * (int)rect[4] / kBpScaleOne);
    rect[5] = y + (sy * (int)rect[5] / kBpScaleOne);
    rect[6] = x + (sx * (int)rect[6] / kBpScaleOne);
    rect[7] = y + (sy * (int)rect[7] / kBpScaleOne);
    if (parentObject != 0) {
        opacity = (opacity * AsBpDrawObject800BC91C(parentObject)->opacity) / kBpOpacityOne;
        packedTintWord = (unsigned int*)Color_ModulatePackedChannels(
            (int)packedTintWord, ((unsigned int)AsBpDrawObject800BC91C(parentObject)->tintColor));
    }
    // Phase 2: combine parent appearance and build source-corner bounds in matching order.
    int scaledOpacityProduct = opacity * opacityScale;
    unsigned int sourceWidth = ((unsigned int)AsBpDrawObject800BC91C(self)->sourceWidth);
    unsigned int sourceLeft = ((unsigned int)AsBpDrawObject800BC91C(self)->sourceX);
    unsigned int sourceCorners[8];
    sourceCorners[0] = sourceLeft;
    unsigned int sourceRight = sourceLeft + sourceWidth;
    unsigned int* drawCornerView = rect;
    unsigned int sourceTop = ((unsigned int)AsBpDrawObject800BC91C(self)->sourceY);
    unsigned int sourceHeight = ((unsigned int)AsBpDrawObject800BC91C(self)->sourceHeight);
    int effectiveOpacity = scaledOpacityProduct / kBpOpacityOne;
    unsigned int* sourceCornerView = sourceCorners;
    unsigned int sourceBottom = sourceTop + sourceHeight;
    sourceCorners[1] = sourceTop;
    sourceCorners[2] = sourceLeft;
    sourceCorners[3] = sourceBottom;
    sourceCorners[4] = sourceRight;
    sourceCorners[5] = sourceBottom;
    sourceCorners[6] = sourceRight;
    sourceCorners[7] = sourceTop;
    // Phase 3: merge only blend bits 4..7 and format bits 12..15, then submit synchronously.
    unsigned int* formatChannel = ((unsigned int*)AsBpDrawObject800BC91C(self)->formatProperty);
    unsigned int* blendChannel = ((unsigned int*)AsBpDrawObject800BC91C(self)->blendProperty);
    unsigned int formatWord = formatChannel[1];
    unsigned int blendWord = blendChannel[1];
    unsigned int flags = (blendWord & kDrawBlendBits) | (formatWord & kDrawFormatBits);
    unsigned int* image = ((unsigned int*)AsBpDrawObject800BC91C(self)->image);
    ((TexturedQuadSubmit)drawCallback)(
        image, drawCornerView, sourceCornerView, effectiveOpacity, packedTintWord,
        flags); // original passes flags in r8 (rlwimi r8 at 0x800BE114), no formatChannel
    return 0;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Transform a colored quad and submit its opacity and packed format/blend flags.
// Same draw-coordinate and fixed-point units as the textured path above.
// Preserve the existing parent case: alpha uses opacityScale * parent.opacity /128,
// then multiplies by opacityScale AGAIN. This differs from the textured path.
// Callback order: source/color block, corners, opacity, combined flags, full format word.
int BpSprite_DrawQuad(void* self, int parentObject, int x, int y, int sx, int sy, int opacityScale) {
    unsigned int* drawCallback = ((unsigned int*)AsBpDrawObject800BC91C(self)->drawQuadCallback);
    int opacity = AsBpDrawObject800BC91C(self)->opacity;
    drawCallback = (drawCallback != 0) ? drawCallback : (unsigned int*)BpCtrl_GetColoredQuadCallback();
    unsigned int rect[8];
    BpSprite_ComputeRotatedQuad(rect, self, parentObject);
    int* r = (int*)rect;
    r[0] = x + (sx * (int)rect[0] / kBpScaleOne);
    r[1] = y + (sy * (int)rect[1] / kBpScaleOne);
    r[2] = x + (sx * (int)rect[2] / kBpScaleOne);
    r[3] = y + (sy * (int)rect[3] / kBpScaleOne);
    r[4] = x + (sx * (int)rect[4] / kBpScaleOne);
    r[5] = y + (sy * (int)rect[5] / kBpScaleOne);
    r[6] = x + (sx * (int)rect[6] / kBpScaleOne);
    r[7] = y + (sy * (int)rect[7] / kBpScaleOne);
    int alpha = opacity;
    if (parentObject != 0)
        alpha = (opacityScale * (AsBpDrawObject800BC91C(parentObject)->opacity)) / kBpOpacityOne;
    unsigned int* formatChannel = ((unsigned int*)AsBpDrawObject800BC91C(self)->formatProperty);
    unsigned int* blendChannel = ((unsigned int*)AsBpDrawObject800BC91C(self)->blendProperty);
    int effectiveOpacity = (alpha * opacityScale) / kBpOpacityOne;
    unsigned int drawFlags = formatChannel[1] & kDrawFormatBits;
    drawFlags |= blendChannel[1] & kDrawBlendBits;
    int (*callback)() = (int (*)())drawCallback;
    ((ColoredQuadSubmit)callback)(&AsBpDrawObject800BC91C(self)->sourceX, rect, effectiveOpacity, drawFlags,
                                  formatChannel[1]);
    return 0;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Apply channel hold/reset controls and queue animation data; negative results report failure.
// self is a slot index encoded as int (callers sometimes pass it as void*), type a
// channel ID 0..17. Third word combines script time with hold/preserve control bits;
// time units are unresolved here. Fourth word is the borrowed channel-value payload
// address, fifth is the command's separate flags word. This wrapper passes both to
// the external node constructor; retention/copying of payload is not established here.
// Return 0 on enqueue OR a suppressed held command, -1 for invalid type, -2 for append
// failure. The slot index must be valid before entering; it is not checked here.
int BpFile_SetAnimation(int self, int type, unsigned int timeAndControl, unsigned int* channelPayload,
                        unsigned int commandFlags) {
    int index = self;
    if (g_bpCommandSlots == 0) {
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    }
    unsigned int offset = index * kBpSlotBytes;
    signed char* state = ((BpCommandSlot*)(g_bpCommandSlots + offset))->channelControl;
    if (type < 0 || type >= kChannelCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(-1, b2dSourceFileName, 0x100,
                                                                               b2dAnimationChannelDiagnostic);
        return -1;
    }
    // Phase 1: a held channel suppresses commands until selection clears its held bit.
    unsigned char bits = state[type];
    if (bits & kControlHold) {
        if (bits & kControlHeld)
            return 0;
        if (timeAndControl & kAnimationHoldRequest) {
            state[type] = bits | kControlHeld;
            return 0;
        }
    }
    // Phase 2: one-shot reset preserves the prior channel value on the next append.
    if (bits & kControlReset) {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        void* obj = (void*)((BpCommandSlot*)(g_bpCommandSlots + offset))->selectedObject;
        BpController_ResetAnimationState(obj, type);
        timeAndControl |= kAnimationPreserveValue;
        state[type] &= ~kControlReset;
    }
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    void* obj = (void*)((BpCommandSlot*)(g_bpCommandSlots + offset))->selectedObject;
    // Phase 3: forward time/control, payload address, and flags as three separate ABI words.
    int result =
        BpCtrl_AppendAnimationChannelNode(index, obj, type, timeAndControl, (unsigned int)channelPayload, commandFlags);
    if (result < 0)
        return -2;
    return 0;
}
#pragma use_lmw_stmw reset

// Optionally request resets on every channel, then forward the script payload value.
int BpScript_ResetChannelsAndForwardValue(int index, unsigned int* args) {
    if (args[3] != 0) {
        if (index < 0 || index >= kBpSlotCount) {
            ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(-1, b2dSourceFileName, 0x62,
                                                                                   b2dFlagControlAllDiagnostic);
        } else {
            if (g_bpCommandSlots == 0)
                g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
            unsigned char* entry = (unsigned char*)(g_bpCommandSlots + index * kBpSlotBytes);
            entry[0x210] |= 1;
            entry[0x211] |= 1;
            entry[0x212] |= 1;
            entry[0x213] |= 1;
            entry[0x214] |= 1;
            entry[0x215] |= 1;
            entry[0x216] |= 1;
            entry[0x217] |= 1;
            entry[0x218] |= 1;
            entry[0x219] |= 1;
            entry[0x21A] |= 1;
            entry[0x21B] |= 1;
            entry[0x21C] |= 1;
            entry[0x21D] |= 1;
            entry[0x21E] |= 1;
            entry[0x21F] |= 1;
            entry[0x220] |= 1;
            entry[0x221] |= 1;
        }
    }
    B2dInp_SetUserData(index, args[4]);
    return 0;
}

// Clear the selected object ID and remove the reset request from all channels.
int BpSlot_ClearSelectionAndControlBit0(int index) {
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    int offset = index * kBpSlotBytes;
    char* base = (char*)g_bpCommandSlots;
    ((BpCommandSlot*)(base + offset))->selectedId = -1;
    if (index < 0 || index >= kBpSlotCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(-1, b2dSourceFileName, 0x62,
                                                                               b2dFlagControlAllDiagnostic);
    } else {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        char* base2 = (char*)g_bpCommandSlots;
        signed char* state = ((BpCommandSlot*)(base2 + offset))->channelControl;
        state[0] &= ~kControlReset;
        state[1] &= ~kControlReset;
        state[2] &= ~kControlReset;
        state[3] &= ~kControlReset;
        state[4] &= ~kControlReset;
        state[5] &= ~kControlReset;
        state[6] &= ~kControlReset;
        state[7] &= ~kControlReset;
        state[8] &= ~kControlReset;
        state[9] &= ~kControlReset;
        state[10] &= ~kControlReset;
        state[11] &= ~kControlReset;
        state[12] &= ~kControlReset;
        state[13] &= ~kControlReset;
        state[14] &= ~kControlReset;
        state[15] &= ~kControlReset;
        state[16] &= ~kControlReset;
        state[17] &= ~kControlReset;
    }
    return 1;
}

// Resolve script data for a flag-3 object and initialize its parent channel after creation.
int BpCmd_CreateObjectFlag3(int index, unsigned int* args) {
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    int offset = index * kBpSlotBytes;
    char* base = (char*)g_bpCommandSlots;
    ((BpCommandSlot*)(base + offset))->selectedId = args[2];
    unsigned int resultOut = args[6];
    if (index < 0 || index >= kBpSlotCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
            -1, (const char*)&b2dSourceFileName, 0x90, (const char*)&b2dFileLabelHookNames);
        resultOut = 0;
    } else {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        base = (char*)g_bpCommandSlots;
        unsigned int block = ((BpCommandSlot*)(base + offset))->fileBlock;
        if (block == 0) {
            ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
                -2, (const char*)&b2dSourceFileName, 0x98, (const char*)&b2dFileLabelHookNames);
            resultOut = 0;
        } else {
            resultOut = ((BpFileBlock*)block)->dataOffset + (resultOut + block);
        }
    }
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    base = (char*)g_bpCommandSlots;
    unsigned int result =
        (unsigned int)BpCtrl_CreateDrawObjectFlag3(index, ((BpCommandSlot*)(base + offset))->selectedId, resultOut);
    if ((int)result >= 0) {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        base = (char*)g_bpCommandSlots;
        void* object = BpSlot_FindItemById(index, ((BpCommandSlot*)(base + offset))->selectedId);
        ((BpObjectParentChannel*)object)->parentId = args[9];
        ((BpObjectParentChannel*)object)->previousParentId = args[9];
    }
    return ((int)result >> 31) & (int)result;
}

// Resolve script data for a flag-2 object and initialize its parent channel after creation.
int BpCmd_CreateObjectFlag2(int index, unsigned int* args) {
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    int offset = index * kBpSlotBytes;
    char* base = (char*)g_bpCommandSlots;
    ((BpCommandSlot*)(base + offset))->selectedId = args[2];
    unsigned int payloadAddress = args[6];
    if (index < 0 || index >= kBpSlotCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
            -1, (const char*)&b2dSourceFileName, 0x90, (const char*)&b2dFileLabelHookNames);
        payloadAddress = 0;
    } else {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        base = (char*)g_bpCommandSlots;
        unsigned int block = ((BpCommandSlot*)(base + offset))->fileBlock;
        if (block == 0) {
            ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
                -2, (const char*)&b2dSourceFileName, 0x98, (const char*)&b2dFileLabelHookNames);
            payloadAddress = 0;
        } else {
            payloadAddress = ((BpFileBlock*)block)->dataOffset + (payloadAddress + block);
        }
    }
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    base = (char*)g_bpCommandSlots;
    int result = BpCtrl_CreateDrawObjectFlag2(index, ((BpCommandSlot*)(base + offset))->selectedId, args[3], args[4],
                                              args[5], payloadAddress);
    if (result >= 0) {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        base = (char*)g_bpCommandSlots;
        void* object = BpSlot_FindItemById(index, ((BpCommandSlot*)(base + offset))->selectedId);
        ((BpObjectParentChannel*)object)->parentId = args[9];
        ((BpObjectParentChannel*)object)->previousParentId = args[9];
    }
    return ((int)result >> 31) & (int)result;
}

#pragma push
#pragma use_lmw_stmw on
// Queue the five channel/data pairs described by the static animation-command table.
int BpFile_SetAnimationBatch(void* self, BpAnimationBatchCommand* args) {
    BF0FCTable table = kBpAnimationBatchChannelMap;
    unsigned int* pair = table.values;
    int i = 0;
    unsigned int value = args->time;
    unsigned int flags = args->flags;
    do {
        int result =
            BpFile_SetAnimation((int)self, pair[0], value, (unsigned int*)((char*)args + pair[1] * 4 + 8), flags);
        if (result < 0) {
            return -1;
        }
        ++i;
        pair += 2;
    } while (i < 5);
    return 0;
}
#pragma pop

// Queue the immediate channel-11 operand with zero flags and return -1 on failure, otherwise zero.
// The channel's visual property is not identified yet.
int BpScript_QueueImmediateChannel11(void* self, BF2FCArgs* args) {
    return BpFile_SetAnimation((int)self, kChannelUnidentified11, 0, &args->operand0, 0) >> 31;
}

// Queue X and Y positions; distinguish a failure on the first or second channel.
int BpScript_QueuePosition(void* self, BpPairAnimationCommand* args) {
    unsigned int value = args->time;
    unsigned int flags = args->flags;
    int result = BpFile_SetAnimation((int)self, kChannelPositionX, value, &args->x, flags);
    if (result < 0) {
        return -1;
    }
    result = BpFile_SetAnimation((int)self, kChannelPositionY, value, &args->y, flags);
    return result < 0 ? -2 : 0;
}

// Queue X and Y scales; distinguish a failure on the first or second channel.
int BpScript_QueueScale(void* self, BpPairAnimationCommand* args) {
    unsigned int value = args->time;
    unsigned int flags = args->flags;
    int result = BpFile_SetAnimation((int)self, kChannelScaleX, value, &args->x, flags);
    if (result < 0) {
        return -1;
    }
    result = BpFile_SetAnimation((int)self, kChannelScaleY, value, &args->y, flags);
    return result < 0 ? -2 : 0;
}

int BpCtrl_RemoveObject(int index, unsigned int value);
int BpCtrl_CreateDrawObjectFromData(int index, unsigned int value, unsigned int* out, unsigned int a, unsigned int b,
                                    unsigned int c);
void* BpSlot_FindItemById(int index, unsigned int value);
extern unsigned int g_b2dTextureSubcommandCallback;
extern const char b2dSourceFileName[];
extern const char b2dTextureSubcommandDiagnostic[];
extern const char b2dFileLabelHookNames[];

// Create the selected object from script data and initialize its parent-ID channel.
int BpCmd_CreateObjectFromData(int index, unsigned int* args) {
    unsigned int saved = args[3];
    if (g_bpCommandSlots == 0) {
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    }
    int offset = index * kBpSlotBytes;
    unsigned int* entry = (unsigned int*)(g_bpCommandSlots + offset);
    entry[0x208 / 4] = args[2];
    if (g_bpCommandSlots == 0) {
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    }
    entry = (unsigned int*)(g_bpCommandSlots + offset);
    unsigned int resultOut = saved;
    unsigned int result =
        BpCtrl_CreateDrawObjectFromData(index, entry[0x208 / 4], &resultOut, args[4], args[5], args[6]);
    if ((int)result >= 0) {
        if (g_bpCommandSlots == 0) {
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        }
        entry = (unsigned int*)(g_bpCommandSlots + offset);
        void* object = BpSlot_FindItemById(index, entry[0x208 / 4]);
        ((BpObjectParentChannel*)object)->parentId = args[9];
        ((BpObjectParentChannel*)object)->previousParentId = args[9];
    }
    return ((int)result >> 31) & (int)result;
}

typedef void (*BE454Callback)(int, unsigned int, unsigned int*);
// Resolve the slot-relative payload and invoke the registered texture command callback.
int B2dFile_DispatchTextureSubcommand(int index, unsigned int* args) {
    if (g_b2dTextureSubcommandCallback == 0) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
            -1, (const char*)&b2dSourceFileName, 0x13B, (const char*)&b2dTextureSubcommandDiagnostic);
        return -1;
    }
    unsigned int payloadOffset = args[2];
    unsigned int offset;
    unsigned int* payloadAddress;
    if (index < 0 || index >= kBpSlotCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
            -1, (const char*)&b2dSourceFileName, 0x90, (const char*)&b2dFileLabelHookNames);
        payloadAddress = 0;
    } else {
        if (g_bpCommandSlots == 0) {
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        }
        offset = index * kBpSlotBytes;
        unsigned int* entry = (unsigned int*)(g_bpCommandSlots + offset);
        unsigned int block = entry[1];
        if (block == 0) {
            ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(
                -2, (const char*)&b2dSourceFileName, 0x98, (const char*)&b2dFileLabelHookNames);
            payloadAddress = 0;
        } else {
            payloadAddress = (unsigned int*)(((BpFileBlock*)block)->dataOffset + block + payloadOffset);
        }
    }
    ((BE454Callback)g_b2dTextureSubcommandCallback)(index, args[3], payloadAddress);
    return 0;
}

// Remove the object currently selected by the command slot.
int BpCtrl_RemoveIndexedObject(int index) {
    if (g_bpCommandSlots == 0) {
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    }
    unsigned int* entry = (unsigned int*)(g_bpCommandSlots + index * kBpSlotBytes);
    BpCtrl_RemoveObject(index, entry[0x208 / 4]);
    return 0;
}

// Queue column/row counts using timeAndControl for the columns channel.
int BpScript_QueueTileCounts(void* self, BpTileAnimationCommand* args) {
    int result = BpFile_SetAnimation((int)self, kChannelTileColumns, args->timeAndControl, &args->columns, 0);
    if (result < 0) {
        return -1;
    }
    result = BpFile_SetAnimation((int)self, kChannelTileRows, 0, &args->rows, 0);
    return result < 0 ? -2 : 0;
}

// Queue an opacity command and return zero or a negative failure indicator.
int BpScript_QueueOpacity(void* self, BpScalarAnimationCommand* args) {
    return BpFile_SetAnimation((int)self, kChannelOpacity, args->time, &args->value, args->flags) >> 31;
}

// Queue a rotation command and return zero or a negative failure indicator.
int BpScript_QueueRotation(void* self, BpScalarAnimationCommand* args) {
    return BpFile_SetAnimation((int)self, kChannelRotation, args->time, &args->value, args->flags) >> 31;
}

// Queue a tint command and return zero or a negative failure indicator.
int BpScript_QueueTintColor(void* self, BpScalarAnimationCommand* args) {
    return BpFile_SetAnimation((int)self, kChannelTint, args->time, &args->value, args->flags) >> 31;
}

// Set both current and previous X/Y pivot values to avoid interpolation from an old pivot.
int BpScript_SetPivot(int index, BpPivotCommand* args) {
    if (g_bpCommandSlots == 0) {
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    }
    unsigned int* entry = (unsigned int*)(g_bpCommandSlots + index * kBpSlotBytes);
    unsigned int target = entry[0x20C / 4];
    (*(unsigned int*)&AsBpDrawObject800BC91C(target)->pivotX) = args->x;
    ((BpPivotChannelDefaults*)target)->pivotX = args->x;
    (*(unsigned int*)&AsBpDrawObject800BC91C(target)->pivotY) = args->y;
    ((BpPivotChannelDefaults*)target)->pivotY = args->y;
    return 0;
}

// Reset enabled channels, clear their hold state, and cache the newly selected object.
int BpSlot_SelectItemAndResetControlFlags(int index, BF2FCArgs* args) {
    if (!g_bpCommandSlots) {
        g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
    }
    int offset = index * kBpSlotBytes;
    unsigned int* entry = (unsigned int*)(g_bpCommandSlots + offset);

    signed char* state = (signed char*)(entry + kBpChannelControlOffset / 4);
    // Selection policy: all 18 channels retain their values; only enabled channels
    // acquire Reset, and every channel loses Held so the new selection can accept
    // commands. Hold itself stays set. SetAnimation later consumes Reset once and
    // adds PreserveValue to the appended node; selecting an object does not do it here.
    // Phase 1: mark enabled animation channels for reset (two groups of nine).
    register int counter = 0;
    for (int i = 0; i < 2; ++i) {
        signed char bits = state[0];
        if (kControlEnabled & bits)
            state[0] = bits | kControlReset;
        bits = state[1];
        if (bits & kControlEnabled)
            state[1] = bits | kControlReset;
        bits = state[2];
        if (bits & kControlEnabled)
            state[2] = bits | kControlReset;
        bits = state[3];
        if (bits & kControlEnabled)
            state[3] = kControlReset | bits;
        bits = state[4];
        if (kControlEnabled & bits)
            state[4] = bits | kControlReset;
        bits = state[5];
        if (bits & kControlEnabled)
            state[5] = bits | kControlReset;
        bits = state[6];
        if (bits & kControlEnabled)
            state[6] = kControlReset | bits;
        bits = state[7];
        if (bits & kControlEnabled)
            state[7] = kControlReset | bits;
        bits = state[8];
        if (kControlEnabled & bits)
            state[8] = bits | kControlReset;
        counter += 8;
        state = (state + 9);
    }
    if (index < 0 || index >= kBpSlotCount) {
        ((int (*)(int, const char*, int, const char*))BpCtrl_InvokeGlobalHook)(-1, b2dSourceFileName, 0x62,
                                                                               b2dFlagControlAllDiagnostic);
    } else {
        if (0 == g_bpCommandSlots) {
            g_bpCommandSlots = __nwa__FUl(kBpSlotStorageBytes);
        }
        state = (signed char*)(g_bpCommandSlots + offset + kBpChannelControlOffset);
        state[0] &= ~kControlHeld;
        state[1] &= ~kControlHeld;
        state[2] &= ~kControlHeld;
        state[3] &= ~kControlHeld;
        state[4] &= ~kControlHeld;
        state[5] &= ~kControlHeld;
        state[6] &= ~kControlHeld;
        state[7] &= ~kControlHeld;
        state[8] &= ~kControlHeld;
        state[9] &= ~kControlHeld;
        state[10] &= ~kControlHeld;
        state[11] &= ~kControlHeld;
        state[12] &= ~kControlHeld;
        state[13] &= ~kControlHeld;
        state[14] &= ~kControlHeld;
        state[15] &= ~kControlHeld;
        state[16] &= ~kControlHeld;
        state[17] &= ~kControlHeld;
    }
    // Phase 2: publish the new selected ID/object after clearing the held-channel flags.
    __typeof__(args->operand0) _p47 = args->operand0;
    unsigned int selectedId = args->operand0;
    unsigned int selectedIdCopy = args->operand0;
    unsigned int selectedIdCopy2 = args->operand0;
    unsigned int selectedIdCopy3 = args->operand0;
    *(unsigned int*)(entry + 0x208 / 4) = selectedIdCopy3;
    *(unsigned int*)(entry + 0x20C / 4) = (unsigned int)BpSlot_FindItemById(index, selectedId);
    return 0;
}

extern unsigned int g_bpCtrlGlobalHook;
extern unsigned int g_b2dTextureSubcommandCallback;
extern float kFloat_6_2831855;
extern float s_twoPi_800BDF64;
extern float s_bpScriptTwoPi;

// Call the installed global hook when present; the legacy entry forwards incoming registers.
void BpCtrl_InvokeGlobalHook() {
    if (g_bpCtrlGlobalHook != 0) {
        ((void (*)())g_bpCtrlGlobalHook)();
    }
}

// Initialize this module's second cached two-pi value.
void BpScript_InitTwoPi() {
    s_bpScriptTwoPi = kFloat_6_2831855;
}

// Install the texture subcommand callback and return success.
int B2d_SetTextureSubcommandCallback(unsigned int value) {
    g_b2dTextureSubcommandCallback = value;
    return 0;
}

// Install the process-wide Bp controller hook.
void BpCtrl_SetGlobalHook(unsigned int value) {
    g_bpCtrlGlobalHook = value;
}

// Return success for an intentionally empty script command.
int BpScript_NoOp() {
    return 0;
}

// Initialize this module's first cached two-pi value.
void BpDraw_InitTwoPi() {
    s_twoPi_800BDF64 = kFloat_6_2831855;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046208C", _ctors_u800BDF64_0, BpDraw_InitTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU8046208C", _ctors_u800BDF64_1, BpScript_InitTwoPi);
#pragma pop

} // extern "C"

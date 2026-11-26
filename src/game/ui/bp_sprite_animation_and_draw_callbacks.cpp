// Bp sprite slots: the value-animation keyframe clock, lookups of named and numbered sprites,
// the offset table, the draw-callback setters, and the stream destructor thunks.

#include <decomp/static_init.h>
// Bp sprite animation clock, controller queries, and draw-callback registration.
// Modding lookup (frame rate / layout / player-panel animation):
// - ValueAnimation_AdvanceKeyframe adds ONE integer tick per invocation. Keyframe
//   durations are ticks, not seconds or retraces. bp_layout_animation_and_sprite_draw.cpp schedules calls
//   and seeks channels; changing presentation alone does not rescale this clock.
// - s_valueAnimationDurationScale, the data section: leading binary32 in an
//   eight-byte slot, default 1.0. CBpCtrl's constructor (bp_ctrl_entry_manager.cpp) selects
//   0.8333333 or 1.0 for duration-scaling policy. Larger values lengthen keys;
//   duration is multiplied then truncated to int. Preserve this FP/integer order.
// - ValueAnimation_SetUpdateMode1/4 affect global scheduling in bp_layout_animation_and_sprite_draw.cpp;
//   they select policy, not a frame rate. Mode 1 resets the shared seek counter;
//   mode 4 advances it once per controller draw. See that unit's mode map.
// - Drawing callbacks are borrowed code addresses, never owned resources. Install
//   compatible signatures; bp_layout_animation_and_sprite_draw.cpp documents the invoked quad interfaces.
// Slot queries return borrowed nodes/user words, valid only while the controller
// and its pool live. Slot handles are 0..39, -1 marks an unused entry. Invalid
// slots report via BpCtrl_InvokeGlobalHook, then return null or leave outputs
// untouched as each wrapper shows. The hook may have its own failure policy.
// String names must be NUL-terminated. All operations share mutable Bp state;
// callers must serialize them with controller traversal/resource teardown.
// Stream destructor thunks are unrelated ABI glue; preserve their receiver
// adjustments, flags, optimization pragmas and forwarding order.

extern "C" {

void IOStream_dtor(void*, int);
void StringStream_dtor(void*, int);
void OStream_dtor(void*, int);
void IStream_DtorThunk(void*, int);
extern void* g_bpDefaultRectDrawCallback;
extern void* g_bpDefaultSolidQuadCallback;
extern void* g_bpCtrlTexturedQuadCallback;
extern void* g_bpColoredQuadDrawCallback;
extern void* g_bpSpriteQuadDrawCallback;
extern void* g_bpDefaultTransformedQuadCallback;
extern void* g_bpDrawObjectCallback;
extern float kFloat_6_2831855;
extern float s_bpSecondaryFullTurn;
extern float kFloat_0_ValueAnimation;
extern float s_valueAnimationDurationScale;
extern float s_bpStartupFullTurn;
extern int g_valueAnimationUpdateMode;
extern signed char s_bpSharedStartupGuard;
extern signed char s_bpSecondaryStartupGuard;
struct BpSlot;
extern BpSlot s_bpObjectEntries[];
extern char kBpControllerSourceFilename[];
extern char bpControllerCallbackDiagnosticStrings[];
extern char bpControllerGetHeadHookNames[];
extern char bpHeadSpriteLookupFunctionName[];
void BpCtrl_InvokeGlobalHook(int, const char*, int, const char*);
int strcmp(void*, void*);

// These are Bp's C controller slots and sprite channels, not CBpObject instances.
enum {
    kBpSlotCount = 40,
    kUnusedBpSlot = -1,
    kMissingBpItem = -1,
    kNullBpName = -2,
    kBpChannelGroups = 6,
    kAnimationSkipReset = 1,
    kInterpolationMask = 0xF,
    kInterpolateLinear = 1,
    kInterpolateHold = 2,
    kInterpolateEaseIn = 3,
    kInterpolateEaseOut = 4,
    kAnimationInteger = 0,
    kAnimationFloat = 1,
    kAnimationColor = 3,
    kTrigOne = 0x1000,
    kTrigQuarterTurn = 0x100,
    kUpdateModeHighBits = 0xF0,
    kVirtualBaseOffsetSlot = 0x3C,
    kSlotRangeDiagnosticLine = 0x84,
    kUnusedSlotDiagnosticLine = 0x89,
    kNullNameDiagnosticLine = 0x159,
    kMissingNodeDiagnosticLine = 0x166,
    kMissingItemDiagnosticLine = 0x17E
};

struct BpKeyframe {
    int valueBits;
    int interpolation;
    int durationFrames; // Historical field spelling: integer animation ticks.
    BpKeyframe* next;
};

// Value words are also interpreted as float or four packed color bytes.
struct BpAnimationChannel {
    int initialValueBits;
    int currentValueBits;
    short valueType;
    short flags;
    int tickPosition; // +0xC tick cursor, also seeded with first duration on reset.
    BpKeyframe* firstKeyframe;
    BpKeyframe* currentKeyframe;
};

// Six storage groups of three, NOT six independent animation properties.
// Flat channel = group*3 + {first:0, second:1, third:2}; channel record starts
// at sprite+0x18+channel*0x18; current value is the word at record+4.
// Property map (BpSpriteChannelIndex in bp_sprite_controller_and_drawing.cpp, draw reads in bp_layout_animation_and_sprite_draw.cpp):
// group  first                 second                 third
// 0      image index (0)       source X (1)           source Y (2)
// 1      source width (3)      source height (4)      clip (5)
// 2      position X (6)       position Y (7)         scale X (8)
// 3      scale Y (9)          opacity (10)           auxiliary (11)
// 4      tile columns (12)    tile rows (13)         pivot X (14)
// 5      pivot Y (15)         rotation (16)          tint (17)
// Source/position are pixels; scales/pivots use 1024 unity, opacity/color 128;
// rotation is signed integer DEGREES (the draw callback consumers convert it).
// Clip/auxiliary names are retained from the producer, with no new tuning domain
// inferred here. Format/blend/tile-flip bits reside in channel metadata, not in
// another interpolated value. Edit a property through its flat channel ID in
// BpFile_SetAnimation (bp_sprite_script_commands.cpp), preserving reset's skip flag per channel.
struct BpChannelGroup {
    BpAnimationChannel first;
    BpAnimationChannel second;
    BpAnimationChannel third;
};

struct BpSpriteNode {
    int reserved;
    int id;
    void* name;
    char pad_0C[8];
    BpSpriteNode* next;
    BpChannelGroup channelGroups[kBpChannelGroups];
};

struct BpSlot {
    int id;
    char pad_04[4];
    BpSpriteNode* items;
    char pad_0C[4];
    int itemCount;
    char pad_14[8];
    int userData1;
    int userData2;
    int opacity;
    char pad_28[8];
    BpSpriteNode* head;
};

// Records store offsets relative to the start of this table, not pointers.
struct OffsetTableRecord {
    int offset;
    char reserved[12];
};
struct OffsetTable {
    int reserved;
    int count;
    char pad_08[8];
    OffsetTableRecord records[1];
};

typedef char BpSlot_size_check[sizeof(BpSlot) == 0x34 ? 1 : -1];
typedef char BpSpriteNode_size_check[sizeof(BpSpriteNode) == 0x1C8 ? 1 : -1];
typedef char BpChannelGroup_size_check[sizeof(BpChannelGroup) == 0x48 ? 1 : -1];

// Borrow the first matching named sprite; return null after reporting invalid slot/name/miss.
void* BpSlot_FindNodeByName(int index, void* name) {
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    void* current;
    if (entry == 0)
        current = 0;
    else
        current = (void*)entry->head;
    if (name == 0) {
        BpCtrl_InvokeGlobalHook(kNullBpName, kBpControllerSourceFilename, kNullNameDiagnosticLine,
                                bpHeadSpriteLookupFunctionName);
        return 0;
    }
    while (current != 0) {
        BpSpriteNode* node = (BpSpriteNode*)current;
        if (node->name != 0 && strcmp(node->name, name) == 0)
            return current;
        current = node->next;
    }
    BpCtrl_InvokeGlobalHook(kMissingBpItem, kBpControllerSourceFilename, kMissingNodeDiagnosticLine,
                            bpHeadSpriteLookupFunctionName);
    return 0;
}

// Restore resettable channels to their first keyframe for all six channel groups.
// A missing default draw callback disables the operation. Bit 0 preserves a
// channel; empty channels stay unchanged. Reset seeds the tick cursor with the
// FIRST key's unscaled duration (not zero), so the next Advance may snap it.
// Neither keys nor sprites are allocated/freed; retain this reset/advance pairing.
void BpSlot_ResetAnimationChannelValues(int index) {
    if (g_bpDefaultRectDrawCallback == 0)
        return;
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    if (entry == 0)
        return;
    BpSpriteNode* node = (BpSpriteNode*)entry->head;
    while (node != 0) {
        BpChannelGroup* item = node->channelGroups;
        for (int i = 0; i < kBpChannelGroups; ++i, ++item) {
            if (item->first.firstKeyframe != 0 && (item->first.flags & kAnimationSkipReset) == 0) {
                item->first.currentValueBits = item->first.firstKeyframe->valueBits;
                item->first.initialValueBits = item->first.firstKeyframe->valueBits;
                item->first.currentKeyframe = item->first.firstKeyframe;
                item->first.tickPosition = item->first.firstKeyframe->durationFrames;
            }
            if (item->second.firstKeyframe != 0 &&
                (item->second.flags & kAnimationSkipReset) == 0) { // 0x800BAF74: existing short at +0x22
                item->second.currentValueBits = item->second.firstKeyframe->valueBits;
                item->second.initialValueBits = item->second.firstKeyframe->valueBits;
                item->second.currentKeyframe = item->second.firstKeyframe;
                item->second.tickPosition = item->second.firstKeyframe->durationFrames;
            }
            if (item->third.firstKeyframe != 0 && (item->third.flags & kAnimationSkipReset) == 0) {
                item->third.currentValueBits = item->third.firstKeyframe->valueBits;
                item->third.initialValueBits = item->third.firstKeyframe->valueBits;
                item->third.currentKeyframe = item->third.firstKeyframe;
                item->third.tickPosition = item->third.firstKeyframe->durationFrames;
            }
        }
        node = node->next;
    }
}

int Trig_GetSinTableEntry(int);

// Advance one animation tick, snapping at the scaled boundary or interpolating a value.
// self and currentKeyframe are required; caller stops calling when cursor is null.
// Writes tick, initial/current value words and key cursor; always returns 0, NOT
// completion. Snap discards overshoot and advances at most one key. Zero scaled
// duration snaps immediately; do not replace this with a fractional-time loop.
// Supported low interpolation nibble: 1 linear, 2 hold, 3/4 sine ease. Other
// nibbles leave a zero divisor on the interpolation path: assets must use valid
// modes. A held key still snaps at its endpoint. Types 0/1/3 mean signed int,
// binary32 word, and four bytes in stored order; other types only advance time.
// Fixed weights are /4096; sine indices cover a quarter turn in 256 entries.
// Color bytes are interpolated separately with integer truncation, without
// saturation. Float bit aliases preserve storage and evaluation/rounding order.
// Numeric admission is a caller/asset obligation, not runtime validation:
// durationFrames >= 0, finite scale >= 0, and the ROUNDED binary32 product must
// be < 2147483648.0f (largest admissible positive float is 2147483520.0f).
// Require tickPosition < INT_MAX before increment. On the interpolating path,
// linear integer/color weight*delta and integer target-initial must fit signed
// 32 bits; sine modes also require 0<=nextTick<=2097151 before nextTick<<10.
// No saturation/overflow recovery exists. Zero product snaps before division.
int ValueAnimation_AdvanceKeyframe(BpAnimationChannel* self) {
    // Phase 1: increment the integer clock and publish a completed key's target.
    BpKeyframe* keyframe = self->currentKeyframe;
    int nextTick = self->tickPosition + 1;
    int scaledDurationTicks = (int)((float)keyframe->durationFrames * s_valueAnimationDurationScale);
    self->tickPosition = nextTick;
    if (scaledDurationTicks <= nextTick) {
        self->tickPosition = 0;
        self->initialValueBits = keyframe->valueBits;
        self->currentValueBits = keyframe->valueBits;
        self->currentKeyframe = keyframe->next;
    } else {
        // Phase 2: choose the easing weight, then apply it in the value's domain.
        int weight = 0;
        int weightScale = 0;
        switch ((int)(keyframe->interpolation & kInterpolationMask)) {
        case kInterpolateLinear:
            weight = nextTick;
            weightScale = scaledDurationTicks;
            break;
        case kInterpolateHold:
            return 0;
        case kInterpolateEaseIn:
            weight =
                kTrigOne - Trig_GetSinTableEntry((((nextTick << 10) / scaledDurationTicks) >> 2) + kTrigQuarterTurn);
            weightScale = kTrigOne;
            break;
        case kInterpolateEaseOut:
            weight = Trig_GetSinTableEntry(((nextTick << 10) / scaledDurationTicks) >> 2);
            weightScale = kTrigOne;
            break;
        }
        switch (self->valueType) {
        case kAnimationInteger:
            self->currentValueBits =
                self->initialValueBits + weight * (keyframe->valueBits - self->initialValueBits) / weightScale;
            break;
        case kAnimationFloat:
            *(float*)&self->currentValueBits =
                *(float*)&self->initialValueBits +
                (float)weight * (*(float*)&keyframe->valueBits - *(float*)&self->initialValueBits) / (float)weightScale;
            break;
        case kAnimationColor: {
            unsigned char* keyframeBytes = (unsigned char*)self->currentKeyframe;
            unsigned char* channelBytes = (unsigned char*)self;
            unsigned char component0 = channelBytes[0];
            unsigned char component1 = channelBytes[1];
            unsigned char component2 = channelBytes[2];
            unsigned char component3 = channelBytes[3];
            channelBytes[4] = component0 + weight * ((int)keyframeBytes[0] - component0) / weightScale;
            channelBytes[5] = component1 + weight * ((int)keyframeBytes[1] - component1) / weightScale;
            channelBytes[6] = component2 + weight * ((int)keyframeBytes[2] - component2) / weightScale;
            channelBytes[7] = component3 + weight * ((int)keyframeBytes[3] - component3) / weightScale;
            break;
        }
        }
    }
    return 0;
}

// Borrow record index 0..count-1 at a table-relative byte offset; out-of-range returns null.
// self is required and must contain count complete 16-byte records; stored offsets
// are trusted. No bounds check against the backing file's byte length occurs here.
void* OffsetTable_GetEntry(void* self, int index) {
    if (((OffsetTable*)self)->count <= index)
        return 0;
    if (index < 0)
        return 0;
    int offset = ((OffsetTable*)self)->records[index].offset;
    return (void*)(offset + (int)self);
}

// Return the slot's sprite-list head after validating its handle.
void* BpSlot_GetSpriteListHead(int index) {
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    void* result;
    if (entry == 0)
        result = 0;
    else
        result = (void*)entry->head;
    return result;
}

// Store the pair of application-owned values associated with this slot.
void BpSlot_SetUserData(int index, void* value1, void* value2) {
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    if (entry) {
        entry->userData1 = (int)value1;
        entry->userData2 = (int)value2;
    }
}

// Read either optional application-owned value after validating the slot.
void B2dGetInfoPtr(int index, void** result1, void** result2) {
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    if (entry) {
        if (result1)
            *result1 = (void*)entry->userData1;
        if (result2)
            *result2 = (void*)entry->userData2;
    }
}

// Search the slot's contiguous sprite array for an item ID.
void* BpSlot_FindItemById(int index, int id) {
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    if (!entry)
        return 0;
    int count = entry->itemCount;
    int i = 0;
    while (i < count) {
        BpSpriteNode* item = &entry->items[i];
        if (id == item->id)
            return &entry->items[i];
        ++i;
    }
    BpCtrl_InvokeGlobalHook(id, kBpControllerSourceFilename, kMissingItemDiagnosticLine, bpControllerGetHeadHookNames);
    return 0;
}

// Store slot opacity unchanged; normal Bp unity is 128 (not an 8-bit 255 alpha).
// Invalid slots report and do nothing; this setter neither clamps nor owns value.
void BpCtrl_SetOpacity(int index, int value) {
    BpSlot* entry;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kSlotRangeDiagnosticLine,
                                bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = &s_bpObjectEntries[index];
        if (entry->id == kUnusedBpSlot) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kUnusedSlotDiagnosticLine,
                                    bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    if (entry)
        entry->opacity = value;
}

// Initialize the secondary value-animation full-turn constant.
void BpCtrl_InitSecondaryFullTurn(void) {
    s_bpSecondaryFullTurn = kFloat_6_2831855;
}

// Initialize the full-turn constant and mark both Bp startup guards.
void BpCtrl_InitStartupState(void) {
    s_bpStartupFullTurn = kFloat_6_2831855;
    if (!s_bpSharedStartupGuard)
        s_bpSharedStartupGuard = 1;
    if (s_bpSecondaryStartupGuard)
        return;
    s_bpSecondaryStartupGuard = 1;
}

#pragma optimization_level 1
// Adjust through the virtual-base displacement before forwarding destruction.
// Stream thunk contract for this group (text_format_trees_and_msg_resources.cpp / mw_string_stream_and_text_tags.cpp):
// virtual-base entries require a NONNULL ABI adjustment receiver with a signed
// displacement word at receiver+0x3C; complete object = receiver+word-bias.
// Bias: ostream 8, istream 0xC, iostream 0x14, stringstream 0x4C.
// Secondary-base entries instead receive complete+0xC, subtracting 0xC directly.
// These receivers are compiler subobject/adjustment addresses, never arbitrary
// byte buffers; do not substitute a complete pointer or call a virtual thunk on
// null. The complete destructors repair their shared ios base adjustments.
// Flags are SIGNED destructor modes, not a mask: 0 skips shared virtual-ios
// teardown and keeps storage; -1 includes ios teardown and keeps storage;
// +1 includes teardown and deletes complete storage. Other signed values follow
// the same !=0/>0 tests. Stringstream narrows the flag to signed short first;
// use the conventional -1/0/1 domain to avoid truncation. Thunks forward intact.
void OStream_VirtualBaseDtorThunk(void* self, int destructionFlags) {
    int offset = kVirtualBaseOffsetSlot;
    int virtualBaseDisplacement = *(int*)((char*)self + offset);
    OStream_dtor((char*)self + virtualBaseDisplacement - 8, destructionFlags);
}
#pragma optimization_level reset

#pragma optimization_level 1
// Adjust the stream virtual base before forwarding destruction.
void IStream_VirtualBaseDtorThunk(void* self, int destructionFlags) {
    register int offset = kVirtualBaseOffsetSlot;
    register int virtualBaseDisplacement = *(int*)((char*)self + offset);
    IStream_DtorThunk((char*)self + virtualBaseDisplacement - 0xC, destructionFlags);
}
#pragma optimization_level reset

#pragma optimization_level 1
// Adjust the virtual base to the owning object before forwarding destruction.
void IOStream_VirtualBaseDtorThunk(void* self, int destructionFlags) {
    int offset = kVirtualBaseOffsetSlot;
    int virtualBaseDisplacement = *(int*)((char*)self + offset);
    char* completeStream = (char*)self + virtualBaseDisplacement;
    completeStream -= 0x14;
    IOStream_dtor(completeStream, destructionFlags);
}
#pragma optimization_level reset

#pragma optimization_level 1
// Adjust the virtual base to the owning object before forwarding destruction.
void StringStream_VirtualBaseDtorThunk(void* self, int destructionFlags) {
    int offset = kVirtualBaseOffsetSlot;
    int virtualBaseDisplacement = *(int*)((char*)self + offset);
    StringStream_dtor((char*)self + virtualBaseDisplacement - 0x4C, destructionFlags);
}
#pragma optimization_level reset

// Adjust the fixed base subobject before forwarding destruction.
void IOStream_SecondaryBaseDtorThunk(void* self, int destructionFlags) {
    IOStream_dtor((char*)self - 0xC, destructionFlags);
}

// Adjust the fixed base subobject before forwarding destruction.
void StringStream_SecondaryBaseDtorThunk(void* self, int destructionFlags) {
    StringStream_dtor((char*)self - 0xC, destructionFlags);
}

// Borrow the default rectangle callback address; null disables gated Bp operations.
void BpCtrl_SetDefaultRectDrawCallback(void* value) {
    g_bpDefaultRectDrawCallback = value;
}

// Return the borrowed default rectangle callback address, possibly null.
void* Bp_GetDefaultRectDrawCallback(void) {
    return g_bpDefaultRectDrawCallback;
}

// Borrow the default solid-quad callback; null makes the corresponding draw path fail.
void BpCtrl_SetSolidQuadCallback(void* value) {
    g_bpDefaultSolidQuadCallback = value;
}

// Borrow the platform textured-quad callback address; this setter does not invoke it.
void BpCtrl_SetTexturedQuadCallback(void* value) {
    g_bpCtrlTexturedQuadCallback = value;
}

// Return the borrowed platform textured-quad callback, possibly null.
void* BpCtrl_GetTexturedQuadCallback(void) {
    return g_bpCtrlTexturedQuadCallback;
}

// Borrow the platform colored-quad callback address; signature belongs to its consumer.
void BpCtrl_SetColoredQuadCallback(void* value) {
    g_bpColoredQuadDrawCallback = value;
}

// Return the borrowed platform colored-quad callback, possibly null.
void* BpCtrl_GetColoredQuadCallback(void) {
    return g_bpColoredQuadDrawCallback;
}

// Borrow the rotated-sprite callback; see BpRotatedQuadCallback in bp_layout_animation_and_sprite_draw.cpp.
void BpCtrl_SetSpriteQuadDrawCallback(void* value) {
    g_bpSpriteQuadDrawCallback = value;
}

// Borrow the transformed-quad callback; see BpTransformedQuadCallback in bp_layout_animation_and_sprite_draw.cpp.
void BpCtrl_SetDefaultTransformedQuadCallback(void* value) {
    g_bpDefaultTransformedQuadCallback = value;
}

// Set the dimensionless duration multiplier; negatives leave the previous value.
// Zero is accepted and makes keys snap. No finite/upper-limit check is made;
// callers must keep scaled durations representable as int. Safe tuning range
// beyond the shipped 0.8333333/1.0 choices has not been established.
void ValueAnimation_SetDurationScale(float value) {
    if (value < kFloat_0_ValueAnimation)
        return;
    s_valueAnimationDurationScale = value;
}

// Select mode 1 (seek-counter reset without advancing here); preserve bits 4..7.
void ValueAnimation_SetUpdateMode1(void) {
    g_valueAnimationUpdateMode = (g_valueAnimationUpdateMode & kUpdateModeHighBits) + 1;
}

// Select mode 4 (advance per controller traversal); preserve bits 4..7.
void ValueAnimation_SetUpdateMode4(void) {
    g_valueAnimationUpdateMode = (g_valueAnimationUpdateMode & kUpdateModeHighBits) + 4;
}

// Borrow the generic object-draw callback address; no invocation or ownership transfer.
void BpCtrl_SetDrawObjectCallback(void* value) {
    g_bpDrawObjectCallback = value;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462080", _ctors_u800BA958_0, BpCtrl_InitStartupState);
STATIC_INIT_ENTRY(".CRT$XCU80462080", _ctors_u800BA958_1, BpCtrl_InitSecondaryFullTurn);
#pragma pop

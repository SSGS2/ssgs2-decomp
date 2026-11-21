#include <decomp/static_init.h>
// Equipped Power/Control/Spin and swing-result HUD fades. CTourWindow uses the
// stat accumulator for a temporary character/caddie preview; CSwingResult2d
// owns a Layout base and changes its elements, not the render presentation rate.
// Modding lookup: kSwingResultFadeUpdates starts the countdown; vfunc_9 decrements
// ONCE PER UPDATE, while vfunc_11 converts that countdown to alpha using the
// separate kFloat_10_SwingResult in u8031F3A4_data.cpp. Change both durations
// together in a later mod. At 30 updates/s the baseline ten calls take ~1/3 s;
// rendering at 60 fps alone does not provide a time-correct fade.
// Result IDs 0..6 map to layout elements 1,2,3,4,5,7,6; 0 is the background,
// 8 is optional. Keep that resource ordering, the hidden/fading states and dirty
// publication contract. No viewport, scissor, resolution or texture-format knob.
// Unless noted, self/output pointers are required and borrowed for the call.
// Player stats use byte wrap BEFORE signed upper clamping; do not widen the sum.
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#include "classes/SavePlayerRecord.h"
#include "classes/MsgResourceHolder.h"
#include "classes/WFixedFont.h"
// Borrow the shared document through its established field view; no ownership change.
static inline CSharedDoc* AsCSharedDoc(void* object) { return (CSharedDoc*)object; }
// Translation unit 0x8031F3A4-0x8031FD4C (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".


enum EquipmentAndSwingConstants {
    kEquippedPartCount = 24,
    kMaximumEquippedAttribute = 50,
    kPowerAttribute = 0,
    kControlAttribute = 1,
    kSpinAttribute = 3,
    kNoCaddie = -1,
    kSpecialClubSetInventorySlot = 14,
    kSwingResultHidden = 0,
    kSwingResultFadingIn = 1,
    kSwingResultVisible = 2,
    kSwingResultFadingOut = 3,
    kSwingResultFadeUpdates = 10,
    kSwingResultElementCount = 9,
    kSwingResultBackgroundElement = 0,
    kSwingResultExtraElement = 8,
    kMaximumResultKind = 6,
    kFullOpacity = 255,
    kWhiteRgbColorBias = 256
};

struct EquippedAttributeState {
    SavePlayerRecord* playerRecord;
    char power; // Stored as a byte and explicitly read as signed during accumulation.
    char control;
    char spin;
};

// The save record stores 24 part indices plus auxiliary equipment in a 0x34-byte block per character.
struct SavedCharacterEquipment {
    short partInventoryIndices[kEquippedPartCount];
    char reserved30;
    signed char auxPartInventoryIndex;
    char reserved32[2];
};
struct SavedPartAtByteCursor { char prefix[0x2CE]; short partInventoryIndex; };
struct SavedAuxPartAtCharacterCursor { char prefix[0x2FF]; signed char inventoryIndex; };
struct SavedClubUpgradeAtClubCursor { char prefix[0x4AD]; signed char level; };
struct ClubUpgradeBonuses {
    short lookupKey;
    short power;
    short control;
    short spin;
};
typedef char SavedCharacterEquipment_size[(sizeof(SavedCharacterEquipment) == 0x34) ? 1 : -1];
typedef char EquippedAttributeState_powerOffset[((int)&((EquippedAttributeState*)0)->power == 4) ? 1 : -1];

struct OwnedMessageResource { 
virtual void destroy(int);
};

extern "C" {

extern const float kFloat_6_2831855; // One turn in radians (6.2831855f).
extern float s_fullTurnConstant3368;
extern float s_fullTurnConstant3370;
extern float s_fullTurnConstant3378;
extern float s_fullTurnConstant3380;
extern float s_fullTurnConstant3388;
extern float s_fullTurnConstant3390;
extern const char swingResultLayoutName[]; // Swing-result layout resource name.
extern void Layout_LoadExtended(void*, int, const char*);
extern CSharedDoc* g_sharedDoc;
extern void* ItemInventory_GetClubSetIdAt(void*, int);
extern void Layout_Init(void*, int);
extern void Layout_Destroy(void*, int);
extern void __dl__FPv(void*);
extern char __vt__14CSwingResult2d[];
extern WFixedFont* g_chatMsg;
extern void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementColor(void*, int, int); // Set the element's packed RGBA color.
extern int Layout_Update(void*);
extern const float kFloat_255_SwingResult; // 255.0f: full fade opacity.
extern const float kFloat_10_SwingResult; // 10.0f: alpha denominator in update calls; keep with kSwingResultFadeUpdates.
extern void* ItemInventory_FindCharacterData(void*, void*);
extern void* ItemInventory_GetCharacterIdAt(void*, int);
extern void* ItemInventory_GetPartIdAt(void*, int);
extern void* ItemInventory_GetCaddieIdAt(void*, int);
extern void* ItemInventory_GetAuxPartIdAt(void*, int);
extern void* ItemInventory_FindPartData(void*, void*);
extern void* ItemInventory_FindCaddieData(void*, void*);
extern void* ItemInventory_FindAuxPartData(void*, void*);
extern void* CClubCustomTbl_FindUpgradeEntry(int, int);
extern void Memory_Copy(void*, void*, int);
struct SwingResultLayout {
    char layoutBaseStorage[0x5c];
    int resultKind;
    int showExtraElement;
    int visibilityState; // Hidden / fading in / visible / fading out; see EquipmentAndSwingConstants.
    int fadeFramesRemaining;
    int enabled;
    int needsRedraw;
    void* vtable;
};
typedef char SwingResultLayout_size[(sizeof(SwingResultLayout) == 0x78) ? 1 : -1];
typedef char SwingResultLayout_timerOffset[((int)&((SwingResultLayout*)0)->fadeFramesRemaining == 0x68) ? 1 : -1];
// Accept the mini-game-hole lifecycle stage without extra work; no state changes.
int CMiniGameHole_vfunc_5() { return 1; }
// Accept the mini-game-hole lifecycle stage without extra work; no state changes.
int CMiniGameHole_vfunc_4() { return 1; }
// Accept the mini-game-hole lifecycle stage without extra work; no state changes.
int CMiniGameHole_vfunc_3() { return 1; }
// Accept the mini-game-hole lifecycle stage without extra work; no state changes.
int CMiniGameHole_vfunc_2() { return 1; }
// Accept the mini-game-hole lifecycle stage without extra work; no state changes.
int CMiniGameHole_vfunc_1() { return 1; }
// Initialize this component's angle constant to one turn during static initialization.
void InitFullTurnConstant3368() { s_fullTurnConstant3368 = kFloat_6_2831855; }
// Initialize this component's angle constant to one turn during static initialization.
void InitFullTurnConstant3370() { s_fullTurnConstant3370 = kFloat_6_2831855; }
// Attach a save record to the attribute accumulator and clear its power, control, and spin totals.
// playerRecordAddress is a borrowed Wii pointer in an int ABI slot; it must
// outlive subsequent Compute calls. No save data is modified or released here.
void PlayerStats_InitEquippedAttributeState(void* self, int playerRecordAddress) {
    ((EquippedAttributeState*)self)->playerRecord = (SavePlayerRecord*)playerRecordAddress;
    ((EquippedAttributeState*)self)->power = 0;
    ((EquippedAttributeState*)self)->control = 0;
    ((EquippedAttributeState*)self)->spin = 0;
}
// Sum character, equipped parts, auxiliary part, caddie, and club upgrades; cap power/control/spin at 50.
// state must hold a valid SavePlayerRecord/inventory selection. attributesOut
// requires three writable bytes in Power, Control, Spin order; return is always
// 1, including missing table entries (those contributions are skipped). Lookup
// pointers are borrowed; state totals are overwritten before output is copied.
int PlayerStats_ComputeEquippedAttributeTotals(void* state, void* attributesOut) {
    EquippedAttributeState* self = (EquippedAttributeState*)state;
    self->power = 0; self->control = 0; self->spin = 0;
    // Phase 1: base character, 24 equipped parts and this character's auxiliary part.
    SavePlayerRecord* characterRecord = self->playerRecord;
    void* characterId = ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, (signed char)(unsigned char)characterRecord->characterIndex);
    // As in the game, missing character IDs suppress both part and auxiliary-part bonuses.
    if (characterId != 0) {
        IFF_STRUCT::sChar* characterData = (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, characterId);
        if (characterData != 0) {
            self->power = (char)(int)characterData->stats[kPowerAttribute];
            self->control = (char)(int)characterData->stats[kControlAttribute];
            self->spin = (char)(int)characterData->stats[kSpinAttribute];
        }
        int partSlot = 0;
        int partByteOffset = 0;
        while (partSlot < kEquippedPartCount) {
            signed char characterIndex = self->playerRecord->characterIndex;
            SavePlayerRecord* partRecord = self->playerRecord;
            // Preserve byte cursor addition order to retain the original register allocation.
            int partByteCursor = (int)characterIndex * sizeof(SavedCharacterEquipment);
            partByteCursor = partByteCursor + (partByteOffset + (int)(long)partRecord);
            short partInventoryIndex = ((SavedPartAtByteCursor*)partByteCursor)->partInventoryIndex;
            void* partId = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, partInventoryIndex);
            if (partId != 0) {
                IFF_STRUCT::sPart* partData = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, partId);
                if (partData != 0) {
                    signed char currentPower = self->power;
                    short powerBonus = (short)partData->stats[kPowerAttribute];
                    self->power = (char)(currentPower + powerBonus);
                    signed char currentControl = self->control;
                    short controlBonus = (short)partData->stats[kControlAttribute];
                    self->control = (char)(currentControl + controlBonus);
                    signed char currentSpin = self->spin;
                    short spinBonus = (short)partData->stats[kSpinAttribute];
                    self->spin = (char)(currentSpin + spinBonus);
                }
            }
            partSlot++;
            partByteOffset += sizeof(short);
        }
        void* auxPartRecord = self->playerRecord;
        signed char auxCharacterIndex = ((SavePlayerRecord*)auxPartRecord)->characterIndex;
        void* auxCharacterCursor = (char*)auxPartRecord + (int)auxCharacterIndex * sizeof(SavedCharacterEquipment);
        void* auxPartId = ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, (signed char)(unsigned char)((SavedAuxPartAtCharacterCursor*)auxCharacterCursor)->inventoryIndex);
        if (auxPartId != 0) {
            IFF_STRUCT::sAuxPart* auxPartData = (IFF_STRUCT::sAuxPart*)ItemInventory_FindAuxPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, auxPartId);
            if (auxPartData != 0) {
                signed char currentPower = self->power;
                signed char powerBonus = (signed char)auxPartData->stats[kPowerAttribute];
                self->power = (char)(currentPower + powerBonus);
                signed char currentControl = self->control;
                signed char controlBonus = (signed char)auxPartData->stats[kControlAttribute];
                self->control = (char)(currentControl + controlBonus);
                signed char currentSpin = self->spin;
                signed char spinBonus = (signed char)auxPartData->stats[kSpinAttribute];
                self->spin = (char)(currentSpin + spinBonus);
            }
        }
    }
    // Phase 2: optional caddie and selected club's upgrade bonuses.
    SavePlayerRecord* caddieRecord = self->playerRecord;
    signed char caddieIndex = caddieRecord->caddieIndex;
    if (caddieIndex != kNoCaddie) {
        void* caddieId = ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, caddieIndex);
        if (caddieId != 0) {
            IFF_STRUCT::sCaddie* caddieData = (IFF_STRUCT::sCaddie*)ItemInventory_FindCaddieData(AsCSharedDoc(g_sharedDoc)->itemInventory, caddieId);
            if (caddieData != 0) {
                signed char currentPower = self->power;
                short powerBonus = (short)caddieData->stats[kPowerAttribute];
                self->power = (char)(currentPower + powerBonus);
                signed char currentControl = self->control;
                short controlBonus = (short)caddieData->stats[kControlAttribute];
                self->control = (char)(currentControl + controlBonus);
                signed char currentSpin = self->spin;
                short spinBonus = (short)caddieData->stats[kSpinAttribute];
                self->spin = (char)(currentSpin + spinBonus);
            }
        }
    }
    SavePlayerRecord* clubRecord = self->playerRecord;
    signed char clubSetIndex = clubRecord->clubSetIndex;
    void* clubCursor = (char*)clubRecord + ((int)clubSetIndex << 1);
    signed char upgradeLevel = (signed char)(unsigned char)((SavedClubUpgradeAtClubCursor*)clubCursor)->level;
    void* clubBonuses = CClubCustomTbl_FindUpgradeEntry(clubSetIndex, upgradeLevel);
    if (clubBonuses != 0) {
        signed char currentPower = self->power;
        short powerBonus = ((ClubUpgradeBonuses*)clubBonuses)->power;
        self->power = (char)(currentPower + powerBonus);
        signed char currentControl = self->control;
        short controlBonus = ((ClubUpgradeBonuses*)clubBonuses)->control;
        self->control = (char)(currentControl + controlBonus);
        signed char currentSpin = self->spin;
        short spinBonus = ((ClubUpgradeBonuses*)clubBonuses)->spin;
        self->spin = (char)(currentSpin + spinBonus);
    }
    // Accumulation wraps in byte storage before this signed comparison; there is no lower-bound clamp.
    if ((signed char)(unsigned char)self->power > kMaximumEquippedAttribute) self->power = kMaximumEquippedAttribute;
    if ((signed char)(unsigned char)self->control > kMaximumEquippedAttribute) self->control = kMaximumEquippedAttribute;
    if ((signed char)(unsigned char)self->spin > kMaximumEquippedAttribute) self->spin = kMaximumEquippedAttribute;
    Memory_Copy(attributesOut, &self->power, 3);
    return 1;
}
// Initialize this component's angle constant to one turn during static initialization.
void InitFullTurnConstant3378() { s_fullTurnConstant3378 = kFloat_6_2831855; }
// Initialize this component's angle constant to one turn during static initialization.
void InitFullTurnConstant3380() { s_fullTurnConstant3380 = kFloat_6_2831855; }
// Initialize this component's angle constant to one turn during static initialization.
void InitFullTurnConstant3388() { s_fullTurnConstant3388 = kFloat_6_2831855; }
// Initialize this component's angle constant to one turn during static initialization.
void InitFullTurnConstant3390() { s_fullTurnConstant3390 = kFloat_6_2831855; }
// Accept the result layout's update stage without extra work; no state changes.
int CSwingResult2d_vfunc_10() { return 1; }

// Enable swing-result rendering and request a redraw.
int CSwingResult2d_EnableAndRequestRedraw(SwingResultLayout* self) {
    self->enabled = 1;
    self->needsRedraw = 1;
    return 1;
}

// Hide the result immediately and request a redraw.
int CSwingResult2d_HideImmediately(SwingResultLayout* self) {
    self->visibilityState = kSwingResultHidden;
    self->needsRedraw = 1;
    return 1;
}

// Begin a ten-update fade-in; restart from the transparent endpoint on repeated requests.
int CSwingResult2d_StartFadeIn(SwingResultLayout* self) {
    self->visibilityState = kSwingResultFadingIn;
    self->needsRedraw = 1;
    self->fadeFramesRemaining = kSwingResultFadeUpdates;
    return 1;
}

// Begin a ten-update fade-out if shown/fading; a hidden result stays untouched.
int CSwingResult2d_StartFadeOut(SwingResultLayout* self) {
    if (self->visibilityState != kSwingResultHidden) {
        self->visibilityState = kSwingResultFadingOut;
        self->needsRedraw = 1;
        self->fadeFramesRemaining = kSwingResultFadeUpdates;
    }
    return 1;
}

// Report whether the swing result is visible or fading.
int CSwingResult2d_vfunc_8(SwingResultLayout* self) {
    return self->visibilityState != kSwingResultHidden;
}

// Select one of seven result graphics and whether the additional element is shown.
// Invalid resultKind returns 0 and preserves all fields; nonzero showExtraElement
// is treated as true. A successful selection returns 1 but does not show the HUD.
int CSwingResult2d_SetResultKindAndExtraElement(SwingResultLayout* self, int resultKind, int showExtraElement) {
    if (resultKind < 0 || resultKind > kMaximumResultKind) {
        return 0;
    }
    self->resultKind = resultKind;
    self->needsRedraw = 1;
    self->showExtraElement = showExtraElement;
    return 1;
}

// Compare a club-set ID with the special inventory entry at slot 14.
int ItemInventory_IsSlot14ClubSetId(int clubSetId) {
    void* specialClubSetId = ItemInventory_GetClubSetIdAt(g_sharedDoc->itemInventory, kSpecialClubSetInventorySlot);
    int idDifference = (int)specialClubSetId - clubSetId;
    return idDifference == 0;
}

// Load the swing-result layout using the requested load mode.
// layout is the initialized borrowed result object; mode is forwarded unchanged
// to Layout_LoadExtended. Return 1 means request submitted, not asynchronous
// resource readiness; no layout ownership transfers to the caller.
int CSwingResult2d_LoadOwnLayout(void* layout, int loadMode) {
    Layout_LoadExtended(layout, loadMode, swingResultLayoutName);
    return 1;
}

// Consume one fade update and publish the endpoint on zero; return 1 even if idle.
// Rendering/enabled do not gate this timer. Do not replace this with a delta-time
// decrement without coordinating fade alpha and state transitions in vfunc_11.
int CSwingResult2d_UpdateFadeCountdown(SwingResultLayout* self) {
    int fadeStateOrFrames = self->fadeFramesRemaining;
    if (fadeStateOrFrames != 0) {
        int remainingFrames = fadeStateOrFrames - 1;
        self->needsRedraw = 1;
        self->fadeFramesRemaining = remainingFrames;
        if (remainingFrames == 0) {
            fadeStateOrFrames = self->visibilityState;
            if (fadeStateOrFrames == kSwingResultFadingIn) {
                self->visibilityState = kSwingResultVisible;
            } else if (fadeStateOrFrames == kSwingResultFadingOut) {
                self->visibilityState = kSwingResultHidden;
            }
        }
    }
    return 1;
}

// Initialize the layout base, install the result vtable, and start disabled and hidden.
// Caller supplies sizeof(SwingResultLayout) storage. showExtraElement is not
// initialized here: set the selection with vfunc_4 before enabling a visible HUD.
SwingResultLayout* CSwingResult2d_Ctor(SwingResultLayout* self) {
    Layout_Init(self, 5);
    self->resultKind = 0;
    self->vtable = (void*)__vt__14CSwingResult2d;
    self->visibilityState = kSwingResultHidden;
    self->fadeFramesRemaining = 0;
    self->enabled = 0;
    self->needsRedraw = 0;
    return self;
}

// Destroy the layout base and optionally release the result object.
// Null is accepted; shouldDelete>0 releases storage. Returned self is only an
// ABI echo and may be dangling; no ownership or valid result object is returned.
SwingResultLayout* CSwingResult2d_dtor(SwingResultLayout* self, int shouldDelete) {
    if (self != 0) {
        Layout_Destroy(self, 0);
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Create a message resource through the shared font and retain its owner.
// The font/owner are borrowed; messageResource is owned and released by Dtor's
// virtual destroy(1). The stable volatile self preserves the original store/load
// sequence around the out-parameter call; do not coalesce these stores.
void* MsgResourceHolder_Ctor(void* self) {
    int creationFlags = 0;
    int creationMode = 0;
    int zero = 0;
    volatile void* stableSelf = self;
    void* holder = (void*)stableSelf;
    void* resourceOut = &((MsgResourceHolder*)stableSelf)->messageResource;
    *(int*)&AsMsgResourceHolder(self)->messageResource = zero;
    WFixedFont* font = g_chatMsg;
    void* owner = font->resourceOwner; // original font +0x10.
    AsMsgResourceHolder(self)->resourceOwner = owner;
    WFixedFont_CreateMsgEcbResource(owner, resourceOut, creationFlags, creationMode);
    *(int*)holder = zero;
    return holder;
}

// Add a character base stat and optional short bonus, wrapping the result to one byte.
// attributeIndex must select a valid stat slot (0..4); characterId is borrowed,
// bonusAttributes may be null or must have a short at that slot. Missing character
// data contributes zero. Only the bonus's LOW BYTE is added, then the sum wraps.
int CharacterStats_GetBasePlusBonusByte(int attributeIndex, void* characterId, void* bonusAttributes) {
    IFF_STRUCT::sChar* characterData = (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, characterId);
    unsigned char baseAttribute = 0;
    if (characterData != 0) {
        baseAttribute = characterData->baseStats[attributeIndex];
    }
    int bonusAttribute = 0;
    if (bonusAttributes != 0) {
        bonusAttribute = (unsigned char)(((short*)bonusAttributes)[attributeIndex]);
    }
    return (baseAttribute + bonusAttribute) & 0xff;
}

// Delete the owned message resource and optionally release the holder.
void* MsgResourceHolder_Dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (AsMsgResourceHolder(self)->messageResource != 0) {
            if (AsMsgResourceHolder(self)->messageResource != 0) {
                void* ownedResource = AsMsgResourceHolder(self)->messageResource;
                ((OwnedMessageResource*)ownedResource)->destroy(1);
            }
            AsMsgResourceHolder(self)->messageResource = 0;
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct CSwingResult2dBase { char layoutAndResultStorage[0x74]; };
class CSwingResult2dV : public CSwingResult2dBase {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual int vfunc2C();
    virtual int vfunc30();
    virtual int vfunc34();
};
// Update the base layout and combine all three virtual update-stage results.
// Requires live self. All +0x2C/+0x30/+0x34 stages run even if an earlier stage
// returns zero; the bitwise AND is intentional, not short-circuit Boolean AND.
int CSwingResult2d_vfunc_3(void* self) {
    int updated = Layout_Update(self);
    int result = updated & 1;
    CSwingResult2dV* obj = (CSwingResult2dV*)self;
    result &= obj->vfunc2C();
    result &= obj->vfunc30();
    result &= obj->vfunc34();
    return result;
}

// Refresh result elements only when enabled and dirty, applying fade alpha and the selected graphic.
// Returns 0 with no layout writes when disabled/clean, 1 after publishing dirty
// state. Alpha is truncated to int and upper-clamped only; callers maintain the
// countdown in 0..10. Hide first disables all nine elements before selection.
int CSwingResult2d_RefreshElementColorsAndEnabled(SwingResultLayout* self) {
    // The fade timer updates independently; this flag gates layout writes only.
    if (self->enabled == 0) return 0;
    if (self->needsRedraw == 0) return 0;
    self->needsRedraw = 0;
    float maximumAlpha = kFloat_255_SwingResult;
    float fadeDuration = kFloat_10_SwingResult;
    for (int element = 0; element < kSwingResultElementCount; ++element) {
        Layout_SetElementEnabled(self, element, 0);
        int alpha = kFullOpacity;
        int visibilityState = self->visibilityState;
        if (visibilityState == kSwingResultFadingIn) {
            int fadeProgress = kSwingResultFadeUpdates - self->fadeFramesRemaining;
            alpha = (int)(maximumAlpha * (float)fadeProgress / fadeDuration);
        } else if (visibilityState == kSwingResultFadingOut) {
            int fadeProgress = self->fadeFramesRemaining;
            alpha = (int)(maximumAlpha * (float)fadeProgress / fadeDuration);
        }
        if (alpha > kFullOpacity) alpha = kFullOpacity;
        // Subtracting 256 produces white RGB and the requested low-byte alpha (0xFFFFFF00 | alpha).
        Layout_SetElementColor(self, element, alpha - kWhiteRgbColorBias);
    }
    if (self->visibilityState != kSwingResultHidden) {
        Layout_SetElementEnabled(self, kSwingResultBackgroundElement, 1);
        switch (self->resultKind) {
            case 0: Layout_SetElementEnabled(self, 1, 1); break;
            case 1: Layout_SetElementEnabled(self, 2, 1); break;
            case 2: Layout_SetElementEnabled(self, 3, 1); break;
            case 3: Layout_SetElementEnabled(self, 4, 1); break;
            case 4: Layout_SetElementEnabled(self, 5, 1); break;
            case 5: Layout_SetElementEnabled(self, 7, 1); break;
            case 6: Layout_SetElementEnabled(self, 6, 1); break;
        }
        if (self->showExtraElement != 0) {
            Layout_SetElementEnabled(self, kSwingResultExtraElement, 1);
        }
    }
    return 1;
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625E4", _ctors_u8031F3A4_0, InitFullTurnConstant3368);
STATIC_INIT_ENTRY(".CRT$XCU804625E4", _ctors_u8031F3A4_1, InitFullTurnConstant3370);
STATIC_INIT_ENTRY(".CRT$XCU804625E4", _ctors_u8031F3A4_2, InitFullTurnConstant3378);
STATIC_INIT_ENTRY(".CRT$XCU804625E4", _ctors_u8031F3A4_3, InitFullTurnConstant3380);
STATIC_INIT_ENTRY(".CRT$XCU804625E4", _ctors_u8031F3A4_4, InitFullTurnConstant3388);
STATIC_INIT_ENTRY(".CRT$XCU804625E4", _ctors_u8031F3A4_5, InitFullTurnConstant3390);
#pragma pop

#include <decomp/static_init.h>
#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CPadMemoryMenuLoad.h"
#include "classes/CMsgDraw.h"

struct SaveMenuSlots { char menuPrefix[0x28]; SavePlayerRecord slots[5]; };
struct SaveMenuPlayTime { char prefix[0x48]; int ticks; };
struct PartyHoleOptions {
    char prefix[0x818];
    unsigned char selectedVariants[18];
    char configuredVariants[19];
    char courseCodes[18];
};
// Tutorial pages are resource-defined; subject names are unresolved here.
// page 0: caption tags PRT_004/005; base art 09..0B, alternate 0C..0E, common 0F.
// page 1: caption tags PRT_006/007; base art 10..11, alternate 12..13, common 14..15.
// page 2: caption tag PRT_009; base art 1A..1B, alternate 1C..1D, common 1E.
// page 3: caption tag PRT_008; base art 16..17, alternate 18..19.
// 00..08 and 24 form the shared frame. Element 23 receives the pair (0, page * 31).
// The callee stores this pair at +48/+4C; its visual interpretation is resource-defined.
// Message anchors: 21 primary for pages 0..2, 22 primary for page 3;
// 1F/20 secondary for pages 0/1. Anchors are layout coordinates, not world positions.
enum TutorialIllustrationIds {
    kTutorialPageFirst = 0, kTutorialPageSecond = 1,
    kTutorialPageThird = 2, kTutorialPageFourth = 3, kTutorialPageCount = 4,
    kTutorialFirstSecondaryAnchor = 0x1F, kTutorialSecondSecondaryAnchor = 0x20,
    kTutorialPrimaryAnchor = 0x21, kTutorialFourthPrimaryAnchor = 0x22,
    kTutorialPageStrip = 0x23, kTutorialFrameEnd = 0x24, kTutorialElementCount = 0x25,
    kTutorialPageStripPitch = 31
};
// Each save record occupies 0x828 bytes, with the used flag at menu + 0x28 + slot*stride.
// Layout rows: slot 0..4, feedback 5..9, highlight 10..14; slot 4 is memory load.
// Player flag 5 is cleared for disabled/idle rows; value-state 3 marks unavailable.
enum SaveMenuLayoutIds { kSaveRecordStride = 0x828, kSlotFeedbackBase = 5,
    kSlotHighlightBase = 10, kSlotUnavailableState = 3, kSlotPlayerFlag = 5,
    kSlotDetailsTextGroup = 0x3FD };
enum SaveMenuConstants { kSaveSlotCount = 5, kMemorySaveSlot = 4, kDetailRowCount = 8,
    kPlayTimeHoursText = 0x27, kPlayTimeMinutesText = 0x26, kPlayTimeSecondsText = 0x25,
    kSaveTicksPerSecond = 30, kSaveTicksPerMinute = 1800, kSaveTicksPerHour = 108000 };
#include "classes/CLoadDataMenuInEntry.h"
#define AsCLoadDataMenuInEntry(p) ((CLoadDataMenuInEntry*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

// Translation unit 0x8033B5B8-0x8033CADC (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CTutorialIllust2d.h"
#include <decomp/cw_bitfields.h>

#define AsCTutorialIllust2d(p) ((CTutorialIllust2d*)(p))

extern "C" {

extern void* g_sharedDoc;
extern unsigned char kDartsHoleVariantExclusionRules[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
void* ItemInventory_FindCourseData(void*, unsigned int);
char* __nwa__FUl(unsigned int);
unsigned int appMallocDummy__FUl(unsigned int);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);

extern const float kFloat_6_2831855;
extern float s_tutorialMessageHelperFullTurnAngle;
extern float s_saveMenuHelperFullTurnAngle;
extern float s_tutorialLayoutHelperFullTurnAngle;
extern float s_partyHoleVariantHelperFullTurnAngle;
extern const float kFloat_1_MenuLayout;
extern const char tutorialIllustrationResourceStrings[];
extern char __vt__17CTutorialIllust2d[];

void Layout_LoadExtended(void*, int, const char*);
void* UIGroup_GetOrCreateById(void*, const char*);
void UIGroup_SetEnabled(void*, int);
void CMsgDraw_ResetTextState(void*);
void NumericTextTable_ClearEntryText(void*, int);
int MPBase1_IsComplete(void*);
void MPBase1_Process(void*, float);
void SaveRecordPrefix_ClearCtor(void*);
void CPadMemoryMenuLoad_CopySaveData(void*, void*);
void* SaveData_AcquireInitializedInstance();
void SaveData_UnpackProfileSections(void*, void*, void*);
void Layout_Destroy(void*, int);
void MenuMessage_Dtor(void*, int);
void __dl__FPv(void*);
void Layout_DrawWithOpacity(void*, float);
void MenuMessage_DrawText(void*);
void Layout_Init(void*, int);
void MenuMessage_Ctor(void*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
int Layout_Update(void*);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementFeedbackState(void*, int, int);
void UITextTable_SetInteger(void*, int, int);
void MenuMessage_SetColorMultiplier(void*, int*);
void MenuMessage_SetPosition(void*, int*);
void MenuMessage_SetText(void*, void*);
void MenuMessage_ResetTextState(void*);
void MenuMessage_UpdateReveal(void*);
int Layout_GetElementPosition(void*, int, int*);
void UIGroup_SetEnabled(void*, int);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
int UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void Text_SetContent(void*, void*);
void Layout_SetElementEnabled(void*, int, int);
void* Layout_GetElementSprite(void*, int);
void Sprite_SetTextureRegionSize2D(void*, void*, int);
extern void* tutorialIllustrationMessageTags[];
extern void* g_soundManager;
extern const float kFloat_0_5_TutorialIllustrationSound;
extern const float kFloat_200_TutorialIllustrationSound;
extern const float kFloat_0_TutorialLayout;
extern const char* kSaveSlotDetailGroupNames[];
extern const char* loadDataSlotGroupIds[];
extern const char saveLoadDetailGroupIds[8];

struct AllocatedObject {
    char data[12];
    // Construct an owned menu-message renderer in this allocation.
    AllocatedObject() { MenuMessage_Ctor(this); }
};

// Mark the tutorial dirty and lazily create its two message renderers.
int CTutorialIllust2d_Initialize(void* p) {
    AsCTutorialIllust2d(p)->initialized = 1;
    AsCTutorialIllust2d(p)->layoutDirty = 1;
    if (!AsCTutorialIllust2d(p)->primaryMessage)
        AsCTutorialIllust2d(p)->primaryMessage = new AllocatedObject;
    if (!AsCTutorialIllust2d(p)->secondaryMessage)
        AsCTutorialIllust2d(p)->secondaryMessage = new AllocatedObject;
    return 1;
}

#pragma push
#pragma opt_lifetimes off
// Hide detail/slot groups and clear all play-time text entries.
int CLoadDataMenuInEntry_ClearSlotDetails(void* p) {
    int i;
    const char** names;
    names = (const char**)kSaveSlotDetailGroupNames;
    i = 0;
    do {
        void* item = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, *names);
        UIGroup_SetEnabled(item, 0);
        ++i;
        ++names;
    } while (i < 8);
    CMsgDraw_ResetTextState(AsCLoadDataMenuInEntry(p)->slotDetailsText);
    NumericTextTable_ClearEntryText(AsCLoadDataMenuInEntry(p)->playTimeTextTable, 0x27);
    NumericTextTable_ClearEntryText(AsCLoadDataMenuInEntry(p)->playTimeTextTable, 0x26);
    NumericTextTable_ClearEntryText(AsCLoadDataMenuInEntry(p)->playTimeTextTable, 0x25);
    i = 0;
    names = (const char**)saveLoadDetailGroupIds;
    do {
        void* item = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, *names);
        UIGroup_SetEnabled(item, 0);
        ++i;
        ++names;
    } while ((unsigned)i < 2);
    for (int i = 0; i < 5; ++i) {
        void* item = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, loadDataSlotGroupIds[i]);
        UIGroup_SetEnabled(item, 0);
    }
    return 0;
}

#pragma pop

// Destroy the two message renderers, base layout, and optional object allocation.
void* CTutorialIllust2d_dtor(void* p, int destroy) {
    if (p) {
        AsCTutorialIllust2d(p)->interfaceVtable = __vt__17CTutorialIllust2d;
        void* a = AsCTutorialIllust2d(p)->primaryMessage;
        if (a) {
            MenuMessage_Dtor(a, 1);
            AsCTutorialIllust2d(p)->primaryMessage = 0;
        }
        void* b = AsCTutorialIllust2d(p)->secondaryMessage;
        if (b) {
            MenuMessage_Dtor(b, 1);
            AsCTutorialIllust2d(p)->secondaryMessage = 0;
        }
        AsCTutorialIllust2d(p)->primaryMessage = 0;
        AsCTutorialIllust2d(p)->secondaryMessage = 0;
        Layout_Destroy(p, 0);
        if (destroy > 0)
            __dl__FPv(p);
    }
    return p;
}

class FileIoMenu { public: 
virtual ~FileIoMenu();
};

// Poll the memory-load menu and unpack a successful result into the fifth save slot.
int CLoadDataMenuInEntry_UpdateMemoryLoad(void* p, float dt) {
    char buffer[0x488];
    if (AsCLoadDataMenuInEntry(p)->memoryLoadMenu) {
        if (MPBase1_IsComplete(AsCLoadDataMenuInEntry(p)->memoryLoadMenu)) {
            if (((CPadMemoryMenuLoad*)AsCLoadDataMenuInEntry(p)->memoryLoadMenu)->loadResult == 2) {
                SaveRecordPrefix_ClearCtor(buffer);
                CPadMemoryMenuLoad_CopySaveData(AsCLoadDataMenuInEntry(p)->memoryLoadMenu, buffer);
                void* ctx = SaveData_AcquireInitializedInstance();
                SaveData_UnpackProfileSections(ctx, &((SaveMenuSlots*)p)->slots[kMemorySaveSlot], buffer);
                (*(char*)&AsCLoadDataMenuInEntry(p)->memoryRecordLoaded) = 1;
            }
            if (AsCLoadDataMenuInEntry(p)->memoryLoadMenu) {
                delete (FileIoMenu*)AsCLoadDataMenuInEntry(p)->memoryLoadMenu;
                AsCLoadDataMenuInEntry(p)->memoryLoadMenu = 0;
            }
        } else MPBase1_Process(AsCLoadDataMenuInEntry(p)->memoryLoadMenu, dt);
        return 1;
    }
    return 0;
}

// Select one of four tutorial illustrations and request its alternate art variant.
int CTutorialIllust2d_SelectIllustrationPage(void* p, int state, void* arg) {
    if (state < 0 || state >= kTutorialPageCount)
        return 0;
    if (AsCTutorialIllust2d(p)->illustrationIndex != state) {
        Sound_PlayByName(g_soundManager, tutorialIllustrationResourceStrings + 0x14,
                    0, 0, 0, 0, kFloat_0_5_TutorialIllustrationSound, kFloat_200_TutorialIllustrationSound, kFloat_1_MenuLayout);
    }
    AsCTutorialIllust2d(p)->illustrationIndex = state;
    AsCTutorialIllust2d(p)->layoutDirty = 1;
    (*(void**)&AsCTutorialIllust2d(p)->alternateIllustrationEnabled) = arg;
    return 1;
}

struct X6B { char pad[120]; };
class X6 : public X6B { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual int v2C(); virtual int v30(); virtual int v34(); virtual int v38();
};
// Update the layout and virtual tutorial stages, combining their completion flags.
int CTutorialIllust2d_UpdateLayoutAndCheckStages(void* p) {
    int result = Layout_Update(p) & 1;
    result &= ((X6*)p)->v2C();
    result &= ((X6*)p)->v30();
    if (AsCTutorialIllust2d(p)->primaryMessage && AsCTutorialIllust2d(p)->secondaryMessage)
        ((X6*)p)->v38();
    result &= ((X6*)p)->v34();
    return result;
}

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Position and select visible illustration messages, then advance their text reveal.
int CTutorialIllust2d_UpdatePageMessages(void* p) {
    int buf[2];
    if (AsCTutorialIllust2d(p)->visible) {
        int value = -1;
        int state;
        MenuMessage_SetColorMultiplier(AsCTutorialIllust2d(p)->primaryMessage, &value);
        MenuMessage_SetColorMultiplier(AsCTutorialIllust2d(p)->secondaryMessage, &value);
        state = AsCTutorialIllust2d(p)->illustrationIndex;
        switch (state) {
        case kTutorialPageFirst:
            Layout_GetElementPosition(p, kTutorialPrimaryAnchor, buf);
            MenuMessage_SetPosition(AsCTutorialIllust2d(p)->primaryMessage, buf);
            MenuMessage_SetText(AsCTutorialIllust2d(p)->primaryMessage, tutorialIllustrationMessageTags[0]);
            Layout_GetElementPosition(p, kTutorialFirstSecondaryAnchor, buf);
            MenuMessage_SetPosition(AsCTutorialIllust2d(p)->secondaryMessage, buf);
            MenuMessage_SetText(AsCTutorialIllust2d(p)->secondaryMessage, tutorialIllustrationMessageTags[1]);
            break;
        case kTutorialPageSecond:
            Layout_GetElementPosition(p, kTutorialPrimaryAnchor, buf);
            MenuMessage_SetPosition(AsCTutorialIllust2d(p)->primaryMessage, buf);
            MenuMessage_SetText(AsCTutorialIllust2d(p)->primaryMessage, tutorialIllustrationMessageTags[2]);
            Layout_GetElementPosition(p, kTutorialSecondSecondaryAnchor, buf);
            MenuMessage_SetPosition(AsCTutorialIllust2d(p)->secondaryMessage, buf);
            MenuMessage_SetText(AsCTutorialIllust2d(p)->secondaryMessage, tutorialIllustrationMessageTags[3]);
            break;
        case kTutorialPageThird:
            Layout_GetElementPosition(p, kTutorialPrimaryAnchor, buf);
            MenuMessage_SetPosition(AsCTutorialIllust2d(p)->primaryMessage, buf);
            MenuMessage_SetText(AsCTutorialIllust2d(p)->primaryMessage, tutorialIllustrationMessageTags[4]);
            MenuMessage_ResetTextState(AsCTutorialIllust2d(p)->secondaryMessage);
            break;
        case kTutorialPageFourth:
            Layout_GetElementPosition(p, kTutorialFourthPrimaryAnchor, buf);
            MenuMessage_SetPosition(AsCTutorialIllust2d(p)->primaryMessage, buf);
            MenuMessage_SetText(AsCTutorialIllust2d(p)->primaryMessage, tutorialIllustrationMessageTags[5]);
            MenuMessage_ResetTextState(AsCTutorialIllust2d(p)->secondaryMessage);
            break;
        }
    } else {
        MenuMessage_ResetTextState(AsCTutorialIllust2d(p)->primaryMessage);
        MenuMessage_ResetTextState(AsCTutorialIllust2d(p)->secondaryMessage);
    }
    // Reveal updates run even when hidden; hidden messages have just been reset above.
    MenuMessage_UpdateReveal(AsCTutorialIllust2d(p)->primaryMessage);
    MenuMessage_UpdateReveal(AsCTutorialIllust2d(p)->secondaryMessage);
    return 1;
}
#pragma pop

// Initialize the tutorial helper's full-turn angle cache.
void InitTutorialMessageHelperFullTurnAngle() {
    s_tutorialMessageHelperFullTurnAngle = kFloat_6_2831855;
}

// Initialize the save-menu helper's full-turn angle cache.
void InitSaveMenuHelperFullTurnAngle() {
    s_saveMenuHelperFullTurnAngle = kFloat_6_2831855;
}

// Initialize the tutorial layout helper's full-turn angle cache.
void InitTutorialLayoutHelperFullTurnAngle() {
    s_tutorialLayoutHelperFullTurnAngle = kFloat_6_2831855;
}

// Initialize the party-hole helper's full-turn angle cache.
void InitFullTurnAngleConstant() {
    s_partyHoleVariantHelperFullTurnAngle = kFloat_6_2831855;
}

// Show the tutorial and mark its layout dirty.
int CTutorialIllust2d_Show(void* p) {
    AsCTutorialIllust2d(p)->visible = 1;
    AsCTutorialIllust2d(p)->layoutDirty = 1;
    return 1;
}

// Hide the tutorial, reset its page, and mark its layout dirty.
int CTutorialIllust2d_HideAndResetPage(void* p) {
    AsCTutorialIllust2d(p)->visible = 0;
    AsCTutorialIllust2d(p)->layoutDirty = 1;
    AsCTutorialIllust2d(p)->illustrationIndex = 0;
    return 1;
}

// Return the current tutorial illustration index.
int CTutorialIllust2d_vfunc_6(void* p) {
    return AsCTutorialIllust2d(p)->illustrationIndex;
}

// Accept this optional tutorial lifecycle stage without additional work.
int CTutorialIllust2d_vfunc_9() {
    return 1;
}

// Accept this optional tutorial lifecycle stage without additional work.
int CTutorialIllust2d_vfunc_10() {
    return 1;
}

void appFreeUni__FPv(void*);
void __dt__Q23std12length_errorFv();
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void __dla__FPv(void*);

struct C628StringGuard {
    char* data;
    int* allocator;
    // Free the temporary string array unless ownership has been transferred.
    ~C628StringGuard() { if (data) __dla__FPv(data); }
};

struct C628Ref {
    void* vtable;
    int refs;
    int count;
    char* text;
    // Adopt a string allocation in a reference-counted deleter record.
    C628Ref(char* p) {
        refs = 1;
        count = 1;
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};

} // extern "C"
namespace std {
struct exception {
    void* vtable;
    ~exception();
};
struct logic_error : exception {
    char* text;
    C628Ref* ref;
    // Copy the exception message and transfer its array to shared ownership.
    logic_error(const char* message) {
        vtable = __vt__Q23std11logic_error;
        int allocator;
        *(char*)&allocator = 0;
        unsigned int n = strlen(message) + 1;
        char* allocated = __nwa__FUl((n * 2) - n);
        text = allocated;
        {
            C628StringGuard guard;
            guard.data = allocated;
            guard.allocator = &allocator;
            ref = new C628Ref(allocated);
            guard.data = 0;
        }
        strcpy(text, message);
    }
    ~logic_error();
};
struct length_error : logic_error {
    // Reuse logic_error message ownership and install the length_error vtable.
    length_error(const char* message) : logic_error(message) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
};
}
extern "C" {

struct C628Vector {
    unsigned char* data;
    unsigned int size;
    unsigned int capacity;
    // Start with empty, unallocated candidate-byte storage.
    C628Vector() : data(0), size(0), capacity(0) {}
    // Destroy candidate storage; byte elements have no separate resources.
    ~C628Vector() {
        if (data) {
            size -= (int)((data + size) - data);
            appFreeUni__FPv(data);
        }
    }
};

struct C628Growth {
    unsigned char* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* capacityAddress;
    unsigned int offset;
    // Start a temporary allocation owner tied to the original capacity word.
    C628Growth(unsigned int* p) : data(0), size(0), capacity(0), capacityAddress(p), offset(0) {}
    // Release the buffer left in this guard after the ownership exchange.
    ~C628Growth() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

// Return a reference to the larger unsigned capacity increment.
inline const unsigned int& C628Max(const unsigned int& a, const unsigned int& b) {
    if (a < b) return b;
    return a;
}

// Count bytes in the half-open candidate range.
inline int C628Distance(const unsigned char* first, const unsigned char* last) {
    return last - first;
}

// Throw the vector length exception using this unit's original exception layout.
inline void C628ThrowError(const char* message) {
    throw std::length_error(message);
}

#pragma push
#pragma use_lmw_stmw on
// Choose a permitted per-hole darts variant after applying course-specific exclusion masks.
// Rules are 11 triples {course code, configured variant, excluded variant bit}.
// A SET bit rejects a candidate. Special course flag low nibble 2 uses bits 3..7,
// shifted down before testing variants 0..2. Codes >= 14 bypass this rule table.
// High course codes map to inventory code-0x80, except sentinel FD; the rule lookup
// still uses the original code. No candidates leaves that hole's selection unchanged.
void PartyRound_ChooseHoleVariants() {
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 1) {
        C628Vector choices;
        for (unsigned char i = 0; i < AsCSharedDoc(g_sharedDoc)->roundHoleCount; ++i) {
            char* base = (char*)g_sharedDoc;
            void* context = base + (int)&((CSharedDoc*)0)->itemInventory;
            unsigned char code = (unsigned char)((PartyHoleOptions*)base)->courseCodes[i];
            // This byte starts as the configured rule selector; below it becomes exclusion bits.
            unsigned char variantRuleOrMask = (unsigned char)((PartyHoleOptions*)base)->configuredVariants[i];
            int high = 0;
            if (code > 0x7f && code != 0xfd) high = 1;
            unsigned int mapped = code;
            if (high) mapped = code - 0x80;
            // Phase 1: resolve inventory metadata, retaining the original code for rule matching.
            char* courseData = (char*)ItemInventory_FindCourseData(context, mapped | 0x28000000u);
            int special = ((((IFF_STRUCT::sCourse*)courseData)->courseFlags & 0xf) == 2);
            // Phase 2: clear the candidate vector and construct the exclusion mask.
            choices.size -= (int)((choices.data + choices.size) - choices.data);
            if (code >= 0xe) {
                variantRuleOrMask = 0;
            } else {
                unsigned char excludedVariants = 0;
                unsigned char* row = kDartsHoleVariantExclusionRules;
                for (int j = 11; j != 0; --j, row += 3) {
                    if (row[0] == code && row[1] == variantRuleOrMask)
                        excludedVariants |= 1 << row[2];
                }
                if (special) excludedVariants = (excludedVariants >> 3) & 0x1f;
                variantRuleOrMask = excludedVariants;
            }
            // Phase 3: append each unmasked variant. Growth below only manages byte storage.
            unsigned int index;
            for (unsigned int k = 0; (index = (unsigned char)k) < 3; k = (unsigned char)k + 1) {
                if (!((1 << index) & variantRuleOrMask)) {
                    unsigned int size = choices.size;
                    unsigned int limit = choices.capacity;
                    if (size < limit) {
                        ++choices.size;
                        choices.data[choices.size - 1] = k;
                    } else {
                        if ((unsigned int)(-1 - choices.capacity) < 1) {
                            C628ThrowError("vector length error");
                        }
                        // Allocate, construct the appended byte, copy old bytes, then exchange ownership.
                        C628Growth next(&choices.capacity);
                        unsigned int amount = (choices.size + 1) - choices.capacity;
                        unsigned int growth2;
                        unsigned int growth1;
                        unsigned int capacity = choices.capacity;
                        if (amount > (unsigned int)(-1 - capacity)) {
                            throw std::length_error("vector length error");
                        }
                        if (capacity < 0x55555555u) {
                            growth1 = ((capacity + 1) * 3) / 5;
                            capacity += C628Max(growth1, amount);
                        } else if (capacity < 0xaaaaaaaau) {
                            growth2 = (capacity + 1) / 2;
                            capacity += C628Max(growth2, amount);
                        } else {
                            capacity = -1;
                        }
                        next.data = (unsigned char*)appMallocDummy__FUl(capacity);
                        next.capacity = capacity;
                        next.offset = choices.size;
                        memset(next.data + choices.size + next.size, (unsigned char)k, 1);
                        unsigned char* old = choices.data;
                        int oldSize = C628Distance(old, choices.data + choices.size);
                        ++next.size;
                        next.offset -= oldSize;
                        memcpy(next.data + next.offset, old, oldSize);
                        memset(old, 0, oldSize);
                        unsigned int newSize = next.size + oldSize;
                        unsigned char* oldData = choices.data;
                        unsigned int oldCapacity = choices.capacity;
                        choices.data = next.data;
                        choices.capacity = next.capacity;
                        next.capacity = oldCapacity;
                        next.data = oldData;
                        choices.size = newSize;
                        next.size = 0;
                    }
                }
            }
            // Phase 4: choose a candidate using rand() modulo the count (one RNG call per nonempty hole).
            unsigned int count = choices.size;
            if (count) {
                unsigned int random = rand();
                ((PartyHoleOptions*)g_sharedDoc)->selectedVariants[i] = choices.data[(unsigned char)(random % count)];
            }
        }
    }
}
#pragma pop

// Rebuild enabled tutorial elements only when initialized and dirty.
int CTutorialIllust2d_RefreshPageElements(void* p) {
    if (!AsCTutorialIllust2d(p)->initialized)
        return 0;
    if (!AsCTutorialIllust2d(p)->layoutDirty)
        return 0;
    // Phase 1: consume the dirty flag and clear all element visibility.
    AsCTutorialIllust2d(p)->layoutDirty = 0;
    for (int i = 0; i < kTutorialElementCount; ++i)
        Layout_SetElementEnabled(p, i, 0);
    if (AsCTutorialIllust2d(p)->visible) {
    // Phase 2: enable the shared frame/strip, then the selected page's art variant.
    for (int i = 0; i <= 8; ++i)
        Layout_SetElementEnabled(p, i, 1);
    Layout_SetElementEnabled(p, kTutorialFrameEnd, 1);
    void* obj = Layout_GetElementSprite(p, kTutorialPageStrip);
    float values[2];
    values[0] = kFloat_0_TutorialLayout;
    int idx = AsCTutorialIllust2d(p)->illustrationIndex;
    int scaled = idx * kTutorialPageStripPitch;
    values[1] = (float)scaled;
    Sprite_SetTextureRegionSize2D(obj, values, idx);
    Layout_SetElementEnabled(p, kTutorialPageStrip, 1);
    int state = AsCTutorialIllust2d(p)->illustrationIndex;
    switch (state) {
    case kTutorialPageFirst:
        Layout_SetElementEnabled(p, 0x0f, 1);
        if (AsCTutorialIllust2d(p)->alternateIllustrationEnabled) {
            Layout_SetElementEnabled(p, 0x0c, 1);
            Layout_SetElementEnabled(p, 0x0d, 1);
            Layout_SetElementEnabled(p, 0x0e, 1);
        } else {
            Layout_SetElementEnabled(p, 9, 1);
            Layout_SetElementEnabled(p, 10, 1);
            Layout_SetElementEnabled(p, 11, 1);
        }
        break;
    case kTutorialPageSecond:
        Layout_SetElementEnabled(p, 0x14, 1);
        Layout_SetElementEnabled(p, 0x15, 1);
        if (AsCTutorialIllust2d(p)->alternateIllustrationEnabled) {
            Layout_SetElementEnabled(p, 0x12, 1);
            Layout_SetElementEnabled(p, 0x13, 1);
        } else {
            Layout_SetElementEnabled(p, 0x10, 1);
            Layout_SetElementEnabled(p, 0x11, 1);
        }
        break;
    case kTutorialPageThird:
        Layout_SetElementEnabled(p, 0x1e, 1);
        if (AsCTutorialIllust2d(p)->alternateIllustrationEnabled) {
            Layout_SetElementEnabled(p, 0x1c, 1);
            Layout_SetElementEnabled(p, 0x1d, 1);
        } else {
            Layout_SetElementEnabled(p, 0x1a, 1);
            Layout_SetElementEnabled(p, 0x1b, 1);
        }
        break;
    case kTutorialPageFourth:
        if (AsCTutorialIllust2d(p)->alternateIllustrationEnabled) {
            Layout_SetElementEnabled(p, 0x18, 1);
            Layout_SetElementEnabled(p, 0x19, 1);
        } else {
            Layout_SetElementEnabled(p, 0x16, 1);
            Layout_SetElementEnabled(p, 0x17, 1);
        }
        break;
    }
    }
    return 1;
}

// Load the tutorial layout with the requested layout-loading mode.
int CTutorialIllust2d_LoadLayout(void* a, int b) {
    Layout_LoadExtended(a, b, tutorialIllustrationResourceStrings);
    return 1;
}

// Draw the tutorial layout and any allocated message renderers.
int CTutorialIllust2d_Draw(void* p) {
    Layout_DrawWithOpacity(p, kFloat_1_MenuLayout);
    void* a = AsCTutorialIllust2d(p)->primaryMessage;
    if (a)
        MenuMessage_DrawText(a);
    void* b = AsCTutorialIllust2d(p)->secondaryMessage;
    if (b)
        MenuMessage_DrawText(b);
    return 1;
}

// Initialize the tutorial layout interface and clear its visibility/message state.
void* CTutorialIllust2d_Ctor(void* p) {
    Layout_Init(p, 5);
    AsCTutorialIllust2d(p)->illustrationIndex = 0;
    AsCTutorialIllust2d(p)->interfaceVtable = __vt__17CTutorialIllust2d;
    AsCTutorialIllust2d(p)->alternateIllustrationEnabled = 0;
    AsCTutorialIllust2d(p)->visible = 0;
    AsCTutorialIllust2d(p)->initialized = 0;
    AsCTutorialIllust2d(p)->layoutDirty = 0;
    (*(int*)&AsCTutorialIllust2d(p)->primaryMessage) = 0;
    (*(int*)&AsCTutorialIllust2d(p)->secondaryMessage) = 0;
    return p;
}

// Copy slot highlight rectangles and disable slots without saved records.
int CLoadDataMenuInEntry_ResetSlotLayout(void* p) {
    struct Entry { char pad[0x28]; unsigned short value; char rest[0x7fe]; };
    float buffer[4];
    int j;
    for (j = 0; j < 5; ++j) {
        Layout_ReadElementRect(AsCLoadDataMenuInEntry(p)->layout, j, buffer);
        Layout_WriteElementRect(AsCLoadDataMenuInEntry(p)->layout, j + kSlotHighlightBase, buffer);
        Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenuInEntry(p)->layout, j + kSlotFeedbackBase, 0);
        Layout_SetElementPlayerFlag(AsCLoadDataMenuInEntry(p)->layout, j + kSlotHighlightBase, 0, kSlotPlayerFlag);
    }
    j = 0;
    do {
        int hasRecord;
        if (j < 0 || j >= 5) {
            hasRecord = 0;
        } else {
            hasRecord = (((Entry*)p)[j].value != 0);
        }
        if (!hasRecord) {
            Layout_SetElementPlayerFlag(AsCLoadDataMenuInEntry(p)->layout, j, 0, kSlotPlayerFlag);
            Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, j, kSlotUnavailableState);
            Layout_SetElementPlayerFlag(AsCLoadDataMenuInEntry(p)->layout, j + kSlotHighlightBase, 0, kSlotPlayerFlag);
            Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, j + kSlotHighlightBase, kSlotUnavailableState);
        }
        ++j;
    } while (j < 4);
    Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, 4, kSlotUnavailableState);
    Layout_SetElementFeedbackState(AsCLoadDataMenuInEntry(p)->layout, 4, kSlotUnavailableState);
    return 1;
}

// Convert saved 30-Hz play ticks into nonnegative hour/minute/second fields.
int CLoadDataMenuInEntry_ShowSlotPlayTime(void* p, int index) {
    char* entry = (char*)p + index * kSaveRecordStride;
    int ticks = ((SaveMenuPlayTime*)entry)->ticks;
    int hours = ticks / kSaveTicksPerHour;
    if (hours <= 0) hours = 0;
    int minutes = ticks / kSaveTicksPerMinute - hours * 60;
    if (minutes <= 0) minutes = 0;
    int secondsOrDetailIndex = ticks / kSaveTicksPerSecond - hours * 3600 - minutes * 60;
    if (secondsOrDetailIndex <= 0) secondsOrDetailIndex = 0;
    UITextTable_SetInteger(AsCLoadDataMenuInEntry(p)->playTimeTextTable, kPlayTimeHoursText, hours);
    UITextTable_SetInteger(AsCLoadDataMenuInEntry(p)->playTimeTextTable, kPlayTimeMinutesText, minutes);
    UITextTable_SetInteger(AsCLoadDataMenuInEntry(p)->playTimeTextTable, kPlayTimeSecondsText, secondsOrDetailIndex);
    // Keep the original variable reuse: splitting it changes CodeWarrior register allocation.
    secondsOrDetailIndex = 0;
    const char** name = (const char**)saveLoadDetailGroupIds;
    for (; (unsigned)secondsOrDetailIndex < 2; ++secondsOrDetailIndex, ++name) {
        void* item = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, *name);
        UIGroup_SetEnabled(item, 1);
    }
    return 1;
}

struct B6Record { char pad[0x28]; unsigned short value; char rest[0x7fe]; };
// Test the slot used flag, returning false for an out-of-range slot.
inline int B6HasRecord(void* p, int index) {
    if (index < 0 || index >= 5) return 0;
    return ((B6Record*)p)[index].value != 0;
}
// Detail rows begin at menu + slot*0x828 + 0x2F4 and have stride 0x34.
// CodeWarrior allocates these bitfields from the high bit: value tests byte mask 0x40.
// The signed one-bit field reads as -1 when set; only its nonzero truth value is used.
struct B6Row {
    CW_BITS3(
        unsigned char pad : 1,
        signed char value : 1,
        unsigned char rest : 6)
    char padding[0x33];
};
// Read one detail row visibility bit after validating the slot and row indices.
inline int B6Enabled(B6Row* row, int index, int i) {
    if (index < 0 || index >= 5) return 0;
    if (i < 0 || i >= 8) return 0;
    return row->value != 0;
}
// Big-endian output word is RRGGBBAA. Incoming tint bytes are 00BBGGRR;
// the shifts reverse RGB byte order. Opacity 0 stays 0; otherwise alpha = 2*opacity-1.
// The layout provider supplies the range; this routine does not clamp it.
union B6Color {
    unsigned int word;
    struct { CW_BITS4( unsigned int r:8, unsigned int g:8, unsigned int b:8, unsigned int a:8) } channels;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Refresh the selected save slot's detail groups, tint, content and play time.
// Returns 0 for invalid indices AND populated-slot refresh; 1 for an empty-slot refresh.
// This is an existing status convention, not a generic success boolean. All groups are borrowed.
int CLoadDataMenuInEntry_UpdateSlotDetails(void* p, int index) {
    unsigned int tintOrSlotIndex;
    void* widget;
    int offset;
    void* item;
    int i;
    if (index < 0 || index >= 5)
        return 0;
    int hasRecord;
    if (index < 0 || index >= 5) {
        hasRecord = 0;
    } else {
        unsigned short value = AsCLoadDataMenuInEntry((char*)p + index * kSaveRecordStride)->firstSlotUsed;
        hasRecord = (value != 0);
    }
    if (!hasRecord) {
        CLoadDataMenuInEntry_ClearSlotDetails(p);
        const char** names;
        int k;
        names = (const char**)loadDataSlotGroupIds;
        for (k = 0; k < 5; ++k, ++names) {
            void* group = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, *names);
            UIGroup_SetEnabled(group, k == index);
        }
        return 1;
    }
    offset = index * kSaveRecordStride;
    // Phase 1: expose the eight per-record detail rows.
    struct RowContainer { char pad[0x2f4]; B6Row rows[8]; };
    char* base = (char*)p + offset;
    item = ((RowContainer*)base)->rows;
    for (int j = 0; j < 8; ++j, item = (B6Row*)item + 1) {
        void* group = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, kSaveSlotDetailGroupNames[j]);
        UIGroup_SetEnabled(group, B6Enabled((B6Row*)item, index, j));
    }
    // Phase 2: rebuild the text using the layout group tint and opacity.
    widget = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, (const char*)kSlotDetailsTextGroup);
    if (widget) {
        CMsgDraw_ResetTextState(AsCLoadDataMenuInEntry(p)->slotDetailsText);
        CMsgDraw_ClearFormattingAndRevealAll(AsCLoadDataMenuInEntry(p)->slotDetailsText);
        if (widget) {
            tintOrSlotIndex = UIGroup_GetTintColor(widget);
            int opacity = UIGroup_GetCombinedOpacity(widget);
            unsigned int c = tintOrSlotIndex;
            unsigned int a = opacity ? (unsigned int)(opacity * 2 - 1) : 0;
            B6Color rgb;
            rgb.word = c << 24;
            rgb.channels.g = c >> 8;
            rgb.channels.b = c >> 16;
            (*(unsigned int*)&((CMsgDraw*)AsCLoadDataMenuInEntry(p)->slotDetailsText)->colorMultiplier) = a | rgb.word;
        }
        Text_SetContent(AsCLoadDataMenuInEntry(p)->slotDetailsText, (char*)p + offset + 0x28);
    }
    // Phase 3: update time text and select exactly one slot group.
    CLoadDataMenuInEntry_ShowSlotPlayTime(p, index);
    for (tintOrSlotIndex = 0; (int)tintOrSlotIndex < 5; ++tintOrSlotIndex) {
        void* group = UIGroup_GetOrCreateById(AsCLoadDataMenuInEntry(p)->bpObject, loadDataSlotGroupIds[tintOrSlotIndex]);
        UIGroup_SetEnabled(group, tintOrSlotIndex == index);
    }
    return 0;
}
#pragma pop

#pragma push
#pragma opt_propagation off
// Finish delayed slot feedback and restore the appropriate selection/highlight states.
int CLoadDataMenuInEntry_UpdateSelectionDelay(void* p) {
    struct Entry { char pad[0x28]; unsigned short value; char rest[0x7fe]; };
    float buffer[4];
    int pending = AsCLoadDataMenuInEntry(p)->selectionDelayFrames;
    if (pending == -1) return 0;
    int remaining = pending - 1;
    AsCLoadDataMenuInEntry(p)->selectionDelayFrames = remaining;
    if (remaining > 0) {
        return 1;
    }
    int item = AsCLoadDataMenuInEntry(p)->selectedItem;
    AsCLoadDataMenuInEntry(p)->selectionDelayFrames = -1;
    if (item != -1) (*(char*)&AsCLoadDataMenuInEntry(p)->selectionComplete) = 1;
    else item = 4;
    if (item < 0 && item > 4) return 1;
    if ((!AsCLoadDataMenuInEntry(p)->memoryRecordLoaded && item == 4) || B6HasRecord(p, item)) {
        Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenuInEntry(p)->layout, item, 1);
        Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenuInEntry(p)->layout, item + kSlotFeedbackBase, 0);
        Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, item + kSlotHighlightBase, 0);
        Layout_ReadElementRect(AsCLoadDataMenuInEntry(p)->layout, item, buffer);
        Layout_WriteElementRect(AsCLoadDataMenuInEntry(p)->layout, item + kSlotHighlightBase, buffer);
    }
    Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, item + kSlotFeedbackBase, 0);
    Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, item + kSlotHighlightBase, 0);
    Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenuInEntry(p)->layout, item + kSlotFeedbackBase, 0);
    if (AsCLoadDataMenuInEntry(p)->selectedItem == -1 && AsCLoadDataMenuInEntry(p)->memoryRecordLoaded) {
        Layout_SetElementValueState(AsCLoadDataMenuInEntry(p)->layout, item, 0);
    }
    return 1;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046265C", _ctors_u8033B5B8_0, InitTutorialMessageHelperFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046265C", _ctors_u8033B5B8_1, InitSaveMenuHelperFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046265C", _ctors_u8033B5B8_2, InitTutorialLayoutHelperFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046265C", _ctors_u8033B5B8_3, InitFullTurnAngleConstant);
#pragma pop

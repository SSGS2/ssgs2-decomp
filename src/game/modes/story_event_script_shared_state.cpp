// Story event scripts: the shared string and value state that script commands read and write,
// the enemy and player-slot setup they call, checks on saved equipment, and text-command cursors.
// Integer comparison helpers used by the scripts are here too.

#include <decomp/static_init.h>
#include <decomp/offset_of.h>
#include "classes/MWStringStorage.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CSharedDoc.h"
#include "classes/CStoryConte.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/vector.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CPangyaEnemyData.h"
extern "C" {

extern float kFloat_6_2831855;
extern float s_storyHelpersFullTurnRadians;
extern float s_storyCommandsFullTurnRadians;
extern float s_sharedStringStateFullTurnRadians;

// String setters copy NUL-terminated input. Getters borrow character storage until
// the corresponding setter, reserve, or destructor runs; callers must not free it.
// Getter pairIndex 0 selects primaryStrings and 1 selects secondaryStrings;
// First/Second select array element 0/1. Context is borrowed and never destroyed here.
// Two primary/secondary string pairs plus the event-script selection values.
struct SharedStringStateStorage {
    MWStringStorage primaryStrings[2];
    MWStringStorage secondaryStrings[2];
    unsigned int valueTag;
    unsigned short valueCode;
    char padding36[2];
    void* context;
    unsigned char characterIndex;
    unsigned char caddieIndex;
};
extern SharedStringStateStorage s_sharedStringState;

enum StoryConstants {
    kSecondaryStringsOffset = (int)DECOMP_OFFSETOF(SharedStringStateStorage, secondaryStrings[0]),
    kPlayerSlots = 4,
    kPlayerRecordBytes = 0x828,
    kSlotIndexVectorsOffset = kPlayerSlots * kPlayerRecordBytes,
    kEnemyRenderTableOffset = kSlotIndexVectorsOffset + kPlayerSlots * 12,
    kEnemyRenderRecordBytes = 0xA4,
    kInitialStringCapacity = 32,
    kInlineLengthMask = 0x7F,
    kCostumePartCount = 24,
    kUnsetPartId = 0xFFFF,
    kScriptCommandBytes = 4,
    kScriptBlockFlags = 0xFFFE,
    kStoryTaskBaseOffset = 0x18,
    kCompareEqual = 0,
    kCompareNotEqual = 1,
    kCompareLess = 2,
    kCompareGreater = 3,
    kCompareLessEqual = 4,
    kCompareGreaterEqual = 5
};
struct CharacterEquipmentRecord {
    unsigned short character;
    short costumeParts[kCostumePartCount];
    char remaining[0xE];
};
struct PlayerHoleScores {
    char prefix[0x28];
    unsigned char scores[18];
};

void* __dt__11CStoryConteFv(void*, int);
void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
void* SharedStringState_dtor(void*, int);
void appFreeUni__FPv(void*);
void ScriptByteMap_FreeSubtree(void*, void**);
void __dl__FPv(void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
extern char* g_sharedDoc;
extern signed char s_renderContextInitialized;
extern signed char s_pangyaEnemyDataInitialized;
extern signed char s_saveDataInitialized;
extern char s_renderContext[kEnemyRenderTableOffset];
extern char s_pangyaEnemyData[0x10];
extern char s_saveData[256];
extern char s_renderContextDtorRegistration_80090284[];
extern char s_enemyDataDtorRegistration_80090284[0x10];
extern char s_sharedStringStateDtorRegistration[256];
void SaveData_InitPlayerRecord(void*);

void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void __dt__13RenderContextFv(void*);
void CharacterPreviewTable_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void __dt__16CPangyaEnemyDataFv(void*);
void EnemyData_BuildCharacterSlot(void*, void*, const char*, int);
void SharedDoc_ApplyEnemyRenderRecordToPlayerSlot(int, void*);
void SaveData_Ctor(void*);
int SaveData_CopyCharacterEquipmentRecordAt(void*, void*, int);
unsigned char EventScript_GetCharacterIndex();
void* SaveData_GetCurrentRecord(void*);
int __cntlzw(unsigned int);

// ABI-sized prefix copied from the save record before checking clubSetIndex.
// Most words remain unidentified; their storage is not a field-level equipment model.
// Transfer schema is intentionally wordwise, preserving the original copy widths.
// bytes2C[1] is the signed club-set index at save offset +0x2D. The rest
// of this prefix is copied but is not consulted by the equipment predicate.
struct StateCopy {
    unsigned short words00[16];
    unsigned int words20[3];
    unsigned char bytes2C[0x0D];
    unsigned int words3C[25];
    unsigned int wordsA0[3];
    unsigned int wordsAC[25];
    unsigned int words110[3];
    unsigned short words11C[12];
    unsigned short word134;
    unsigned char bytes136[7];
    unsigned int word140;
    unsigned int words144[25];
    unsigned int word1A8;
    unsigned int word1AC;
    unsigned int word1B0;
    unsigned int word1B4;
    unsigned int word1B8;
    unsigned int word1BC;
    unsigned int word1C0;
    unsigned int word1C4;
    unsigned int word1C8;
    unsigned int word1CC;
    unsigned int word1D0;
    unsigned int word1D4;
    unsigned int word1D8;
    unsigned int word1DC;
    unsigned int word1E0;
    unsigned int word1E4;
    unsigned int word1E8;
    unsigned int word1EC;
    unsigned int word1F0;
    unsigned int word1F4;
    unsigned int word1F8;
    unsigned int word1FC;
    unsigned int word200;
    float floats204[3];
    unsigned int word210;
    float float214;
    unsigned char bytes218[64];
    unsigned int word258;
};

struct TourConteScriptTable {
    const char* scriptNames[11];
};
extern TourConteScriptTable s_tourConteMessageScriptNames;

struct TourConteSelectionTail {
    unsigned short pre[407];
    char conteOrder[8];
    signed char currentConte;
    char rest[5];
};

struct TextState {
    char prefix[0x2];
    unsigned short flags;
    unsigned char extraFlags[2];
    unsigned char count6;
    unsigned char count7;
    char commandState[0x410];
    char* cursor;
    int sourceUpdateCount;
};

// Replace the first string in each primary/secondary pair, preserving the string allocator ABI.
void SharedStringState_SetFirstStrings(int, const char* primaryText, const char* secondaryText, int) {
    char t0;
    char buf1 = t0;
    unsigned int heapBacked = s_sharedStringState.primaryStrings[0].flags >> 31;
    unsigned int length = heapBacked ? s_sharedStringState.primaryStrings[0].length
                                     : *(unsigned char*)&s_sharedStringState & kInlineLengthMask;
    String_ReplaceRange(&s_sharedStringState, 0, length, primaryText, primaryText + strlen(primaryText), &buf1);
    char t1;
    char buf2 = t1;
    // Preserve the independent base-address expressions used by the original string ABI.
    heapBacked = *(unsigned int*)((char*)&s_sharedStringState + kSecondaryStringsOffset) >> 31;
    length = heapBacked ? *(unsigned int*)((char*)&s_sharedStringState + kSecondaryStringsOffset + 4)
                        : *(unsigned char*)((char*)&s_sharedStringState + kSecondaryStringsOffset) & kInlineLengthMask;
    String_ReplaceRange((char*)&s_sharedStringState + kSecondaryStringsOffset, 0, length, secondaryText,
                        secondaryText + strlen(secondaryText), &buf2);
}

struct NameAllocator {
    char c;
};
// Return the empty allocator token expected by the string replacement ABI.
inline NameAllocator CurrentNameAllocator() {
    NameAllocator t0;
    return t0;
}

// destination must already be a valid MW string. Save progression indices must
// select one of the 11 names; the routine trusts both order indices.
// Copy the saved tour progression's selected conte-message script filename into the caller's string.
void SaveData_GetSelectedTourConteScriptName(void* destination) {
    // Phase 1: construct/register/publish the render context, with unwind cleanup.
    if (((int&)Sing_805DC648()) == 0) {
        if (s_renderContextInitialized == 0) {
            char* end = s_renderContext + kSlotIndexVectorsOffset;
            char* p = s_renderContext;
            char* base = s_renderContext;
            do {
                SaveData_InitPlayerRecord(p);
                p += kPlayerRecordBytes;
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            char* obj = base + kEnemyRenderTableOffset;
            __construct_array(obj, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                              kEnemyRenderRecordBytes, 4);
            ((CEnemyRenderTable*)obj)->count = 0;
            try {
                RenderContext_ResetPlayerRecords(base);
            } catch (...) {
                ((void (*)(void*, int))CharacterPreviewTable_dtor)(base + kEnemyRenderTableOffset, -1);
                ((IntVectorArray*)(base + kSlotIndexVectorsOffset))->~IntVectorArray();
                throw;
            }
            __register_global_object((void*)s_renderContext, (void*)__dt__13RenderContextFv,
                                     (void*)&s_renderContextDtorRegistration_80090284);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = (int)s_renderContext;
    }
    // Render-context publication is complete; resolve the saved script and copy it.
    TourConteScriptTable scriptTable = s_tourConteMessageScriptNames;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(&s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)&s_saveData;
    }
    TourConteSelectionTail progression =
        *(TourConteSelectionTail*)((char*)SaveData_GetCurrentRecord((void*)((int&)GetSaveDataSlot())) + 0x4EC);
    const char* scriptName = scriptTable.scriptNames[(signed char)progression.conteOrder[progression.currentConte]];
    unsigned int heapBacked = *(unsigned int*)destination >> 31;
    int length =
        !heapBacked ? *(unsigned char*)destination & kInlineLengthMask : AsMWStringStorage(destination)->length;
    NameAllocator buf = CurrentNameAllocator();
    String_ReplaceRange(destination, 0, length, scriptName, scriptName + strlen(scriptName), &buf.c);
}

#pragma use_lmw_stmw on
// index must identify a valid render/player slot (0..3); name is borrowed for the
// synchronous build. The slot record is stored in the shared render context.
// Initialize render/enemy singletons as needed, build the named enemy and apply it to a player slot.
void SharedDoc_LoadEnemyPlayerSlotByName(int index, const char* name) {
    // Phase 1: publish render context only after all owned records are initialized.
    if (((int&)Sing_805DC648()) == 0) {
        if (s_renderContextInitialized == 0) {
            char* end;
            char* p;
            char* base;
            base = s_renderContext;
            p = base;
            end = base + kSlotIndexVectorsOffset;
            do {
                SaveData_InitPlayerRecord(p);
                p += kPlayerRecordBytes;
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            char* obj = base + kEnemyRenderTableOffset;
            __construct_array(obj, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                              kEnemyRenderRecordBytes, 4);
            ((CEnemyRenderTable*)obj)->count = 0;
            try {
                RenderContext_ResetPlayerRecords(base);
            } catch (...) {
                ((void (*)(void*, int))CharacterPreviewTable_dtor)(base + kEnemyRenderTableOffset, -1);
                ((IntVectorArray*)(base + kSlotIndexVectorsOffset))->~IntVectorArray();
                throw;
            }
            __register_global_object((void*)s_renderContext, (void*)__dt__13RenderContextFv,
                                     (void*)&s_renderContextDtorRegistration_80090284);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = (int)s_renderContext;
    }
    char* obj = (char*)((int&)Sing_805DC648()) + kEnemyRenderTableOffset;
    if (((int&)Sing_805DC678()) == 0) {
        if (s_pangyaEnemyDataInitialized == 0) {
            ((CPangyaEnemyData*)s_pangyaEnemyData)->Init();
            __register_global_object(s_pangyaEnemyData, (void*)__dt__16CPangyaEnemyDataFv,
                                     (void*)s_enemyDataDtorRegistration_80090284);
            s_pangyaEnemyDataInitialized = 1;
        }
        ((int&)Sing_805DC678()) = (int)s_pangyaEnemyData;
    }
    // Both singleton owners now exist; build into the selected slot, then apply it.
    EnemyData_BuildCharacterSlot((char*)((int&)Sing_805DC678()), obj + index * kEnemyRenderRecordBytes, name, 0);
    SharedDoc_ApplyEnemyRenderRecordToPlayerSlot(index, obj);
}
#pragma use_lmw_stmw reset

// Phase notes for lazy owners: construct records/vectors first, register the
// destructor only after successful initialization, then publish the singleton.
// The int-reference casts below are the 32-bit singleton storage ABI.

// Check the selected character's 24 costume-part slots for the requested ID.
int EventScript_IsCostumePartEquipped(int id) {
    CharacterEquipmentRecord state;
    int index = (signed char)EventScript_GetCharacterIndex();
    if (!((int&)GetSaveDataSlot())) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(&s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)&s_saveData;
    }
    if (!SaveData_CopyCharacterEquipmentRecordAt((void*)((int&)GetSaveDataSlot()), &state, index))
        return 0;
    for (unsigned char i = 0; i < kCostumePartCount; ++i) {
        short value = state.costumeParts[i];
        if (value != kUnsetPartId && id == value)
            return 1;
    }
    return 0;
}

// Compare the active save record's club-set index with the requested index.
int EventScript_IsClubSetEquipped(int index) {
    StateCopy state;
    if (!((int&)GetSaveDataSlot())) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(&s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)&s_saveData;
    }
    state = *(StateCopy*)SaveData_GetCurrentRecord((void*)((int&)GetSaveDataSlot()));
    if ((signed char)state.bytes2C[1] != (signed char)index)
        return 0;
    return 1;
}

// Reserve initial capacity in the first primary and secondary shared strings.
void SharedStringState_ReserveFirstStrings() {
    String_ReserveCapacity(&s_sharedStringState, kInitialStringCapacity);
    String_ReserveCapacity(&s_sharedStringState.secondaryStrings[0], kInitialStringCapacity);
}

// node must be nonnull. Nodes contain byte keys without owned payload allocations;
// the expanded traversal frees descendants before the node itself. owner is forwarded.
// Free both children and each byte-map node in postorder; preserve the expanded traversal shape.
// Child links are borrowed during traversal; only the postorder free owns destruction.
struct ByteMapNode {
    void* left;
    void* right;
};
void ScriptByteMap_FreeSubtree(void* owner, void** node) {
    void* tree = owner;
    void** root = node;
    void* branch = 0;
    void* left = 0;
    // Child words are left at +0 and right at +4; neither owns payloads.
    // Phase 1: destroy the left branch, children before parent.
    void* leftRoot = ((ByteMapNode*)node)->left;
    if (leftRoot != 0) {
        left = ((ByteMapNode*)leftRoot)->left;
        if (left != 0) {
            void* branch = ((ByteMapNode*)left)->left;
            if (branch != 0) {
                void* child = ((ByteMapNode*)branch)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)branch)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(branch);
            }
            branch = ((ByteMapNode*)left)->right;
            if (branch != 0) {
                void* child = ((ByteMapNode*)branch)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)branch)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(branch);
            }
            appFreeUni__FPv(left);
        }
        void* branch = ((ByteMapNode*)leftRoot)->right;
        if (branch != 0) {
            void* rightChild = ((ByteMapNode*)branch)->left;
            if (rightChild != 0) {
                void* child = ((ByteMapNode*)rightChild)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)rightChild)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(rightChild);
            }
            rightChild = ((ByteMapNode*)branch)->right;
            if (rightChild != 0) {
                void* child = ((ByteMapNode*)rightChild)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)rightChild)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(rightChild);
            }
            appFreeUni__FPv(branch);
        }
        appFreeUni__FPv(leftRoot);
    }
    // Phase 2: destroy the right branch with the same postorder schedule.
    branch = ((ByteMapNode*)root)->right;
    if (branch != 0) {
        void* rightRoot = ((ByteMapNode*)branch)->left;
        if (rightRoot != 0) {
            void* leftChild = ((ByteMapNode*)rightRoot)->left;
            if (leftChild != 0) {
                void* child = ((ByteMapNode*)leftChild)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)leftChild)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(leftChild);
            }
            leftChild = ((ByteMapNode*)rightRoot)->right;
            if (leftChild != 0) {
                void* child = ((ByteMapNode*)leftChild)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)leftChild)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(leftChild);
            }
            appFreeUni__FPv(rightRoot);
        }
        rightRoot = ((ByteMapNode*)branch)->right;
        if (rightRoot != 0) {
            void* rightChildLeaf = ((ByteMapNode*)rightRoot)->left;
            if (rightChildLeaf != 0) {
                void* child = ((ByteMapNode*)rightChildLeaf)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)rightChildLeaf)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(rightChildLeaf);
            }
            rightChildLeaf = ((ByteMapNode*)rightRoot)->right;
            if (rightChildLeaf != 0) {
                void* child = ((ByteMapNode*)rightChildLeaf)->left;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                child = ((ByteMapNode*)rightChildLeaf)->right;
                if (child != 0)
                    ScriptByteMap_FreeSubtree(tree, (void**)child);
                appFreeUni__FPv(rightChildLeaf);
            }
            appFreeUni__FPv(rightRoot);
        }
        appFreeUni__FPv(branch);
    }
    // Phase 3: both branches are gone; release the caller-supplied root.
    appFreeUni__FPv(root);
}

// Store the event-script caddie selection.
void SharedStringState_SetCaddieIndex(unsigned char value) {
    s_sharedStringState.caddieIndex = value;
}

// Return the event-script caddie selection.
unsigned char EventScript_GetCaddieIndex() {
    return s_sharedStringState.caddieIndex;
}

// Store the shared script's 32-bit value tag.
void SharedStringState_SetWordValue(unsigned int value) {
    s_sharedStringState.valueTag = value;
}

// Store the shared script context pointer.
void SharedStringState_SetContext(void* value) {
    s_sharedStringState.context = value;
}

// Return the shared script context pointer.
void* SharedStringState_GetContext() {
    return s_sharedStringState.context;
}

// Default shared-string-state hook: perform no work.
void SharedStringState_DefaultHook() {}

// Return the first string of pair 0 or 1, or null for an empty string or invalid pair.
void* SharedStringState_GetFirstString(int pairIndex) {
    if (pairIndex != 1) {
        if (pairIndex == 0) {
            unsigned int inlineStorage = (unsigned int)__cntlzw(s_sharedStringState.primaryStrings[0].flags >> 31) >> 5;
            unsigned int length;
            if (inlineStorage)
                length = *(unsigned char*)&s_sharedStringState & kInlineLengthMask;
            else
                length = s_sharedStringState.primaryStrings[0].length;
            if (length == 0)
                return 0;
            if (!inlineStorage)
                return *(void**)&s_sharedStringState.primaryStrings[0].data;
            return (char*)&s_sharedStringState + 1;
        }
        return 0;
    }
    unsigned int inlineStorage = (unsigned int)__cntlzw(s_sharedStringState.secondaryStrings[0].flags >> 31) >> 5;
    unsigned int length;
    if (inlineStorage)
        length = *(unsigned char*)&s_sharedStringState.secondaryStrings[0].flags & kInlineLengthMask;
    else
        length = s_sharedStringState.secondaryStrings[0].length;
    if (length == 0)
        return 0;
    if (!inlineStorage)
        return *(void**)&s_sharedStringState.secondaryStrings[0].data;
    return (char*)&s_sharedStringState.secondaryStrings[0] + 1;
}

// Replace the second string in each primary/secondary pair.
void SharedStringState_SetSecondStrings(void*, const char* primaryText, const char* secondaryText) {
    struct Al {
        char c;
    };
    unsigned int heapBacked = s_sharedStringState.primaryStrings[1].flags >> 31;
    unsigned int length = !heapBacked
                              ? *(unsigned char*)&s_sharedStringState.primaryStrings[1].flags & kInlineLengthMask
                              : s_sharedStringState.primaryStrings[1].length;
    Al t0;
    Al buf1 = t0;
    String_ReplaceRange(&s_sharedStringState.primaryStrings[1].flags, 0, length, primaryText,
                        primaryText + strlen(primaryText), &buf1.c);
    heapBacked = s_sharedStringState.secondaryStrings[1].flags >> 31;
    length = !heapBacked ? *(unsigned char*)&s_sharedStringState.secondaryStrings[1].flags & kInlineLengthMask
                         : s_sharedStringState.secondaryStrings[1].length;
    Al t1;
    Al buf2 = t1;
    String_ReplaceRange(&s_sharedStringState.secondaryStrings[1].flags, 0, length, secondaryText,
                        secondaryText + strlen(secondaryText), &buf2.c);
}

// Return the second string of pair 0 or 1, or null for an empty string or invalid pair.
void* SharedStringState_GetSecondString(int pairIndex) {
    if (pairIndex != 1) {
        if (pairIndex == 0) {
            unsigned int inlineStorage = (unsigned int)__cntlzw(s_sharedStringState.primaryStrings[1].flags >> 31) >> 5;
            unsigned int length;
            if (inlineStorage)
                length = *(unsigned char*)&s_sharedStringState.primaryStrings[1].flags & kInlineLengthMask;
            else
                length = s_sharedStringState.primaryStrings[1].length;
            if (length == 0)
                return 0;
            if (!inlineStorage)
                return *(void**)&s_sharedStringState.primaryStrings[1].data;
            return (char*)&s_sharedStringState.primaryStrings[1] + 1;
        }
        return 0;
    }
    unsigned int inlineStorage = (unsigned int)__cntlzw(s_sharedStringState.secondaryStrings[1].flags >> 31) >> 5;
    unsigned int length;
    if (inlineStorage)
        length = *(unsigned char*)&s_sharedStringState.secondaryStrings[1].flags & kInlineLengthMask;
    else
        length = s_sharedStringState.secondaryStrings[1].length;
    if (length == 0)
        return 0;
    if (!inlineStorage)
        return *(void**)&s_sharedStringState.secondaryStrings[1].data;
    return (char*)&s_sharedStringState.secondaryStrings[1] + 1;
}

// Evaluate the requested integer comparison; the strict-order cases preserve overflow-safe bit arithmetic.
int Int32_CompareByMode(int lhs, int mode, int rhs) {
    switch (mode) {
    case kCompareEqual:
        return lhs == rhs;
    case kCompareNotEqual:
        return lhs != rhs;
    case kCompareLess: {
        int x = rhs ^ lhs;
        int half = x >> 1;
        x &= rhs;
        x = half - x;
        return x < 0;
    }
    case kCompareGreater: {
        int x = lhs ^ rhs;
        int half = x >> 1;
        x &= lhs;
        x = half - x;
        return x < 0;
    }
    case kCompareLessEqual:
        return rhs >= lhs;
    case kCompareGreaterEqual:
        return lhs >= rhs;
    default:
        return 0;
    }
}

// Release the four shared strings in reverse order and optionally delete their storage.
void* SharedStringState_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        MWStringStorage* strings = (MWStringStorage*)self;
        if (&strings[3])
            if (strings[3].flags >> 31)
                appFreeUni__FPv(strings[3].data);
        if (&strings[2])
            if (strings[2].flags >> 31)
                appFreeUni__FPv(strings[2].data);
        if (&strings[1])
            if (strings[1].flags >> 31)
                appFreeUni__FPv(strings[1].data);
        if (self)
            if (strings[0].flags >> 31)
                appFreeUni__FPv(strings[0].data);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// count is the number of hole entries to compare, at most 18. Lower scores win;
// an empty round also returns true because 0 * 3 >= 0.
// Require slot 1 to have no lower stroke counts than slot 0, and higher counts on at least a third of holes.
// Historical shared name: requires no losses as well as the win threshold.
int EventScript_Player0UnbeatenWinsAtLeastOneThird() {
    unsigned char count = AsCSharedDoc(g_sharedDoc)->roundHoleCount;
    unsigned char playerZeroStrokes;
    unsigned char holesPlayerZeroLeads = 0;
    unsigned char i = 0;
    while ((unsigned char)i < count) {
        void* base = GolfPlayer_GetEntryByIndex((void*)AsCSharedDoc(g_sharedDoc)->golfDoc, 0);
        playerZeroStrokes = ((PlayerHoleScores*)base)->scores[(unsigned char)i];
        base = GolfPlayer_GetEntryByIndex((void*)AsCSharedDoc(g_sharedDoc)->golfDoc, 1);
        unsigned char playerOneStrokes = ((PlayerHoleScores*)base)->scores[(unsigned char)i];
        if (playerOneStrokes < playerZeroStrokes)
            return 0;
        if (playerOneStrokes > playerZeroStrokes)
            holesPlayerZeroLeads = (unsigned char)(holesPlayerZeroLeads + 1);
        ++i;
    }
    return (int)(holesPlayerZeroLeads * 3) >= count;
}

// Initialize this story-helper unit's cached two-pi constant.
void StoryHelpers_InitializeTwoPi() {
    s_storyHelpersFullTurnRadians = kFloat_6_2831855;
}

// Initialize this script-command unit's cached two-pi constant.
void StoryCommands_InitializeTwoPi() {
    s_storyCommandsFullTurnRadians = kFloat_6_2831855;
}

// The stream must contain a complete header and NUL terminator. Length excludes NUL;
// (length + 4) & ~3 rounds length + 1 up to four bytes. Returns interpreter success (1).
// Skip a four-byte command header and its null-terminated text, rounding the next cursor to four bytes.
int EventScript_SkipAlignedTextCommand(TextState* self) {
    char* cursor = self->cursor + kScriptCommandBytes;
    unsigned int len = strlen(cursor);
    self->cursor = cursor + ((len + kScriptCommandBytes) & ~3u);
    return 1;
}

// flags bit 0 is allowed; any other flag suppresses counters. Counters wrap at
// 256 and advance only while sourceUpdateCount is zero; header byte 1 selects count7.
// Advance a four-byte command and count its source marker when execution flags allow it.
int EventScript_CountSourceMarkerAndAdvance(TextState* self) {
    char* cursor = self->cursor;
    if (self->flags & kScriptBlockFlags) {
        self->cursor = cursor + kScriptCommandBytes;
        return 1;
    }
    if (self->sourceUpdateCount == 0) {
        unsigned char tmp = (unsigned char)cursor[1];
        unsigned char c6 = self->count6;
        c6 = (unsigned char)(c6 + 1);
        self->count6 = c6;
        if (tmp != 0) {
            self->count7 = (unsigned char)(self->count7 + 1);
        }
    }
    self->cursor = cursor + kScriptCommandBytes;
    return 1;
}

// Zero-initialize the four shared strings and register their global destructor.
void SharedStringState_InitializeGlobals() {
    s_sharedStringStateFullTurnRadians = kFloat_6_2831855;
    s_sharedStringState.primaryStrings[0].flags = 0;
    s_sharedStringState.primaryStrings[0].length = 0;
    s_sharedStringState.primaryStrings[0].data = 0;
    s_sharedStringState.primaryStrings[1].flags = 0;
    s_sharedStringState.primaryStrings[1].length = 0;
    s_sharedStringState.primaryStrings[1].data = 0;
    s_sharedStringState.secondaryStrings[0].flags = 0;
    s_sharedStringState.secondaryStrings[0].length = 0;
    s_sharedStringState.secondaryStrings[0].data = 0;
    s_sharedStringState.secondaryStrings[1].flags = 0;
    s_sharedStringState.secondaryStrings[1].length = 0;
    s_sharedStringState.secondaryStrings[1].data = 0;
    __register_global_object(&s_sharedStringState, (void*)SharedStringState_dtor,
                             (void*)&s_sharedStringStateDtorRegistration);
}

// Store the event-script character selection.
void SharedStringState_SetCharacterIndex(unsigned char value) {
    s_sharedStringState.characterIndex = value;
}

// Return the event-script character selection.
unsigned char EventScript_GetCharacterIndex() {
    return s_sharedStringState.characterIndex;
}

// Return the shared script's 32-bit value tag.
unsigned int SharedStringState_GetWordValue() {
    return s_sharedStringState.valueTag;
}

// Store the shared script's 16-bit value code.
void SharedStringState_SetShortValue(unsigned short value) {
    s_sharedStringState.valueCode = value;
}

// Return the shared script's 16-bit value code.
unsigned short SharedStringState_GetShortValue() {
    return s_sharedStringState.valueCode;
}

} // extern "C"

// Adjust the secondary task interface pointer before forwarding the destructor callback.
void* CStoryConte::Destroy(int shouldDelete) {
    return __dt__11CStoryConteFv((char*)this - kStoryTaskBaseOffset, shouldDelete);
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462050", _ctors_u80090284_0, StoryHelpers_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462050", _ctors_u80090284_1, SharedStringState_InitializeGlobals);
STATIC_INIT_ENTRY(".CRT$XCU80462050", _ctors_u80090284_2, StoryCommands_InitializeTwoPi);
#pragma pop

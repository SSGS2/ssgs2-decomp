// Character-select model previews: saved outfits are translated into avatar equipment selections,
// each preview avatar is created or refreshed, and preview animations and hover tracking are updated.
// Also the render context that owns the previews and the enemy-data shared file loader.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Character-select model previews, save-outfit translation, and render-context
// lifetime. Lookup for mods:
// - Appearance: FillFromSaveData -> BuildItemSelection -> SetupAvatar; inventory
//   indices become IFF type IDs, then equipment is applied to an owned renderer.
// - Framing: InitCharacterPreviewTransforms initializes eight CHARACTER rows;
//   SetupAvatar combines kCharacterPreviewScales, rotations (radians), viewport
//   offsets (pixels). kHoveredCharacterPreviewScale multiplies those scales.
// - Hover: CCharaSelect::UpdateHoveredAvatars maps layout IDs to character indices
//   then save histories to render slots; failure/cache behavior is documented there.
// - Animation names: SetRecordName marks bit 0x4000; Update retries until accepted.
// - Ownership: PlayerRenderRecord_dtor frees renderer/name; table dtor destroys
//   four constructed records. CopyPreservingRenderer is a SHALLOW descriptor copy.
// Shared record/table headers retain historical Enemy ABI names pending a shared
// rename. They represent general character previews here. CPangyaEnemyData is a
// separate, actual enemy-data file interface and keeps its verified class name.
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#include "classes/CCharaSelect.h"
#include "classes/SavePlayerRecord.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CEnemyRenderRecord.h"
#include "classes/CEnemyRenderTable.h"
typedef CEnemyRenderRecord CharacterPreviewRecord;
typedef CEnemyRenderTable CharacterPreviewTable;
#define AsCharacterPreviewRecord(p) ((CharacterPreviewRecord*)(p))
#define AsCharacterPreviewTable(p) ((CharacterPreviewTable*)(p))
#include "game/vector.h"
#include "game/wvector.h"

#include "game/singletons.h"
#include "classes/CPangyaEnemyData.h"
#include <decomp/cw_bitfields.h>
#include "classes/CAsyncExhibition.h"
#include "classes/CCharacterPreviewRecord.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CAsyncExhibitionAvatar.h"

// NAMING NOTE: CEnemyRenderRecord / CEnemyRenderTable (shared class names) are used in this unit for the four
// per-player *character preview* records of the render context (an animated avatar, its animation name, and
// the inventory indices of what it wears), built from save data. These records
// have no enemy-specific behavior; the separate CPangyaEnemyData methods below do.
// ABI names are retained for linkage; local views below use CharacterPreview.
// Flags shared by the record's name/animation update and saved player options.
enum CharacterPreviewFlags {
    kPreviewNameChanged = 0x4000,
    kSavedPreviewFlags = 0x3000
};
enum CharacterPreviewLayout {
    kPlayerCount = 4,
    kCharacterCount = 8,
    kOutfitPartCount = 24,
    kCharacterPreviewRecordSize = sizeof(CharacterPreviewRecord),
    kPlayerRecordSize = sizeof(SavePlayerRecord),
    kRenderVectorsOffset = (int)&((RenderContext*)0)->vectors,
    kRenderPlayersOffset = (int)&((RenderContext*)0)->players,
    kRenderAvatarOffset = kRenderPlayersOffset + (int)&((CharacterPreviewRecord*)0)->renderObject,
    kDefaultAnimationNameOffset = 0x109, // Empty terminator after C_NO_ENT_0.
    kPreviewLeft = 20,
    kPreviewTop = 120,
    kPreviewWidth = 150,
    kPreviewBottom = 400
};
// Preserve the existing byte-addition grouping for indexed render-context storage.
enum CharacterPreviewOffsets {
    kPreviewNameOffset = (int)&((CharacterPreviewRecord*)0)->animationName,
    kPreviewPartInventoryIndicesOffset = (int)&((CharacterPreviewRecord*)0)->partInventoryIndices,
    kPreviewFlagsOffset = (int)&((CharacterPreviewRecord*)0)->flags,
    kClubCustomizationOffset =
        0x60 // club-customization block inside the preview record (filled by ClubCustomization_GetFivePartValues)
};
// Save-file offsets use the save record as origin, independently of preview layout.
enum SavedOutfitLayout {
    kSavedClubSetIndex = 0x2D,
    kSavedBallIndex = 0x2E,
    kSavedCaddieIndex = 0x2F,
    kSavedCaddieItemIndex = 0x30,
    kSavedFirstItemIndex = 0x31,
    kSavedOutfitStride = 0x34,
    kSavedOutfitParts = 0x2CE,
    kSavedOutfitHair = 0x2FE,
    kSavedOutfitAuxiliary = 0x2FF,
    kOutfitGroupCount = 3,
    kPartsPerOutfitGroup = 8,
    kPartGroupBytes = 0x10
};
// Only values consumed by the preview builder are interpreted in this save-record view.
struct SavedCharacterPreviewValue {
    unsigned int firstWord;
    void* value;
};
struct SavedPreviewSource {
    char prefix[0x24];
    void* rankValue;
    char otherSelections[0x444];
    SavedCharacterPreviewValue characterValues[8];
    char otherAttributes[0x3C];
    unsigned int previewOptionBits;
};
// Save -> preview schema (all indices refer to inventory tables, not type IDs):
// +0x2d/+0x2e/+0x2f: signed club/ball/caddie; +0x30: caddie item;
// +0x31..38: eight signed item indices. A negative index represents no item.
// Character idx selects a 0x34-byte outfit at +0x2cc + idx*0x34:
//   +0: gate bits, +2: 24 signed 16-bit part indices, +0x32: signed hair index,
//   +0x33: auxiliary part index. The shared SavePlayerRecord header names the
// first three selections; the outfit region still lives in its padding.
// previewOptionBits bit31 -> preview flags 0x2000, bit30 -> 0x1000. Their
// gameplay meanings are unresolved; 0x4000 independently means name pending.
typedef char SavedPreviewSource_flagsOffset[((int)&((SavedPreviewSource*)0)->previewOptionBits == 0x4E8) ? 1 : -1];
// CCharaSelect's shared header has no field at 0x48 yet; this view names only that pointer.
struct CharaHoverCache {
    char prefix[0x48];
    int* lastHoveredCharacter;
};
typedef char CharacterPreviewRecord_size[(sizeof(CharacterPreviewRecord) == 0xA4) ? 1 : -1];
extern "C" int kCharacterPreviewLayoutElementIds[8];
struct CharacterElementMap {
    int v[8];
};
// Copy the eight layout element IDs used to identify character rows.
static inline CharacterElementMap CopyCharacterElementMap(const int* g) {
    CharacterElementMap m;
    m.v[0] = g[0];
    m.v[1] = g[1];
    m.v[2] = g[2];
    m.v[3] = g[3];
    m.v[4] = g[4];
    m.v[5] = g[5];
    m.v[6] = g[6];
    m.v[7] = g[7];
    return m;
}
// Resolve a hovered layout element ID to its character index, or -1.
static inline int FindCharacterElementIndex(CharacterElementMap m, int id) {
    int characterIndex;
    if (id == m.v[0])
        characterIndex = 0;
    else if (id == m.v[1])
        characterIndex = 1;
    else if (id == m.v[2])
        characterIndex = 2;
    else if (id == m.v[3])
        characterIndex = 3;
    else if (id == m.v[4])
        characterIndex = 4;
    else if (id == m.v[5])
        characterIndex = 5;
    else if (id == m.v[6])
        characterIndex = 6;
    else if (id == m.v[7])
        characterIndex = 7;
    else
        characterIndex = -1;
    return characterIndex;
}
// Resolve a layout element ID against an existing eight-entry map.
static inline int FindCharacterElementIndexInArray(const int* a, int id) {
    int characterIndex;
    if (id == a[0])
        characterIndex = 0;
    else if (id == a[1])
        characterIndex = 1;
    else if (id == a[2])
        characterIndex = 2;
    else if (id == a[3])
        characterIndex = 3;
    else if (id == a[4])
        characterIndex = 4;
    else if (id == a[5])
        characterIndex = 5;
    else if (id == a[6])
        characterIndex = 6;
    else if (id == a[7])
        characterIndex = 7;
    else
        characterIndex = -1;
    return characterIndex;
}

extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u80245F84 {
    char* base;
    bool active;
    // Arm cleanup of constructed context records/vectors until reset succeeds.
    explicit RenderContextConstructionCleanup_u80245F84(void* object) : base((char*)object), active(true) {}
    // Release constructed subobjects only while reset remains incomplete.
    ~RenderContextConstructionCleanup_u80245F84() {
        if (active) {
            CharacterPreviewTable_dtor(base + kRenderPlayersOffset, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + kRenderVectorsOffset + index * 12);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Initialize the render context while guarding its constructed vectors and preview records.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u80245F84 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

extern char __vt__16CPangyaEnemyData[];
extern char s_enemyDataResourcePath[];
extern char s_renderContext[];
extern signed char s_renderContextInitialized;
extern char s_renderContextRegistrations_805041E8[];
extern char kCharacterPreviewLoadingIndicatorPositions[];
extern int kCharacterPreviewLayoutElementIds[8];
void Layout_CollectHovered(void*, void*, int);

void ItemSelection_Init(void*);
int ItemSelection_Validate(void*);
void ItemSelection_RefreshPrimaryItems(void*);
int ItemInventory_FindCharacterIndex(void*, int);
void __as__7WVectorFRC7WVector(void*, void*);
void Vec2_Copy(void*, void*);
extern float kCharacterPreviewScales[];
extern char charaSelectStrings[];
void Memory_Copy(void*, void*, unsigned int);
void* SharedFile_Create(const char*, int);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __destroy_arr(void*, void*, int, int);
void* RenderContext_GetOrCreate(void);
void RenderContext_ResetPlayerRecords(void*);
void SaveData_InitPlayerRecord(void*);

void PartyItemRecord_InitDefaults(void*);
void __dt__13RenderContextFv(void*);
void __register_global_object(void*, void*, void*);
void __construct_array(void*, void*, void*, unsigned int, unsigned int);
void* RenderContext_GetLastSlotRecordIndex(void*, int);
void* RenderContext_GetCurrentSlotRenderRecord(void*, int);
void CharaSelect_UpdateSlotRenderRecord(void*, int, int, bool, int, int);
void* ItemInventory_GetCharacterIdAt(void*, int);
void* ItemInventory_GetHairStyleIdAt(void*, int);
void* ItemInventory_FindHairStyleData(void*, int);
void* ItemInventory_GetPartIdAt(void*, short);
int ItemSelection_BuildCharacterOutfit(void*, void*, int, int, void*, int);
extern void* g_sharedDoc;
extern float kHoveredCharacterPreviewScale;
extern float kFloat_1_CharaSelect;
extern float kFloat_0_CharacterPreview;
extern float kFloat_neg0_CharacterPreview;
extern float kFloat_neg0_2_CharacterPreview;
extern float kFloat_neg0_1_CharacterPreview;
extern float kFloat_neg4_CharacterPreview;
extern float kFloat_neg13_CharacterPreview;
extern float kFloat_5_CharacterPreview;
extern float kFloat_2_CharacterPreview;
extern float kFloat_8_CharacterPreview;
extern float kFloat_7_CharacterPreview;
extern float kFloat_neg3_CharacterPreview;
extern float kFloat_10_CharacterPreview;
extern float kFloat_13_CharacterPreviewStorage;
extern float kFloat_6_2831855;
extern float s_characterPreviewRuntimeTau;
extern float s_characterPreviewRotations[];
extern float s_characterPreviewViewportOffsets[];
int strcmp(void*, const char*);
unsigned int String_ReplaceRange(void*, int, int, const char*, const char*, char*);
unsigned int strlen(const char*);
int PlayerRank_FindTierForValue(void*);
int PlayerRank_FindThresholdBand(void*);
void ClubCustomization_GetFivePartValues(void*, void*, int);

// Replace the record's animation-name string and mark it for application to its avatar.
int CCharacterPreviewTable::SetAnimationName(int index, const char* text) {
    if (index < 0 || count <= index)
        return 0;
    CCharacterPreviewRecord* record = &records[index];
    char* name = (char*)&record->animationName;
    unsigned int flag = *(unsigned int*)name;
    int isInline = !(flag >> 31);
    if (!strcmp((void*)text, isInline ? (char*)record + kPreviewNameOffset + 1 : (*(char**)&record->animationName[2])))
        return 1;
    int oldLength = isInline ? (*(unsigned char*)name & 0x7f) : (*(int*)&record->animationName[1]);
    char temp = 0;
    unsigned int len = strlen(text);
    String_ReplaceRange(name, 0, oldLength, text, text + len, &temp);
    record->flags |= kPreviewNameChanged;
    return 1;
}

// Initialize per-character preview rotations, viewport offsets, and the runtime full-circle constant.
void InitCharacterPreviewTransforms() {
    // Eight character-index rows: rotations are XYZ triples passed unchanged to
    // SetRotation, in radians. CAsyncExhibition::UpdatePointerRotation in
    // async_exhibition_and_pet_format.cpp converts degree-scaled input by pi/180 before the same setter;
    // CExhibition::SetRotation forwards to the Euler matrix builder in exhibition_model_preview.cpp.
    // Viewport rows are {dx,dy} pixels added to both rectangle corners. These
    // tables share character order with kCharacterPreviewScales, not player order.
    float thirteen, ten, m3, seven, eight, two, five, one, m13, m4, m01, m02, nz, z, twoPi;
    float* rotationRows = s_characterPreviewRotations;
    float* viewportOffsetRows = s_characterPreviewViewportOffsets;
    z = kFloat_0_CharacterPreview;
    nz = kFloat_neg0_CharacterPreview;
    m02 = kFloat_neg0_2_CharacterPreview;
    m4 = kFloat_neg4_CharacterPreview;
    one = kFloat_1_CharaSelect;
    ten = kFloat_10_CharacterPreview;
    m01 = kFloat_neg0_1_CharacterPreview;
    m13 = kFloat_neg13_CharacterPreview;
    five = kFloat_5_CharacterPreview;
    two = kFloat_2_CharacterPreview;
    eight = kFloat_8_CharacterPreview;
    seven = kFloat_7_CharacterPreview;
    m3 = kFloat_neg3_CharacterPreview;
    thirteen = kFloat_13_CharacterPreviewStorage;
    twoPi = kFloat_6_2831855;
    rotationRows[0] = z;
    rotationRows[1] = nz;
    rotationRows[2] = z; // character 0
    rotationRows[3] = z;
    rotationRows[4] = m02;
    rotationRows[5] = z; // character 1
    rotationRows[6] = z;
    rotationRows[7] = nz;
    rotationRows[8] = z; // character 2
    rotationRows[9] = z;
    rotationRows[10] = nz;
    rotationRows[11] = z; // character 3
    rotationRows[12] = z;
    rotationRows[13] = nz;
    rotationRows[14] = z; // character 4
    rotationRows[15] = z;
    rotationRows[16] = m02;
    rotationRows[17] = z; // character 5
    rotationRows[18] = z;
    rotationRows[19] = nz;
    rotationRows[20] = z; // character 6
    rotationRows[21] = z;
    rotationRows[22] = m01;
    rotationRows[23] = z; // character 7
    viewportOffsetRows[0] = z;
    viewportOffsetRows[1] = m4; // character 0
    viewportOffsetRows[2] = m13;
    viewportOffsetRows[3] = z; // character 1
    viewportOffsetRows[4] = one;
    viewportOffsetRows[5] = five; // character 2
    viewportOffsetRows[6] = two;
    viewportOffsetRows[7] = z; // character 3
    viewportOffsetRows[8] = one;
    viewportOffsetRows[9] = eight; // character 4
    viewportOffsetRows[10] = seven;
    viewportOffsetRows[11] = m4; // character 5
    viewportOffsetRows[12] = m3;
    viewportOffsetRows[13] = ten; // character 6
    viewportOffsetRows[14] = ten;
    viewportOffsetRows[15] = thirteen; // character 7
    s_characterPreviewRuntimeTau = twoPi;
}

// Copy saved selection and outfit attributes into a character preview without creating its renderer.
void CCharacterPreviewRecord::FillFromSaveData(int characterIndex, void* saveRecord) {
    // The save record is borrowed. The caller supplies a valid character index
    // and a distinct writable record. No renderer is created or released.
    signed char* saveBytes = (signed char*)saveRecord;
    unsigned int flags = ((SavedPreviewSource*)saveRecord)->previewOptionBits;
    unsigned short previewFlags = this->flags;
    // Preserve other preview bits; transfer the two high save-option bits.
    previewFlags = (previewFlags & ~kSavedPreviewFlags) | (((flags >> 30) & 3) << 12);
    int clubSetIndex = saveBytes[kSavedClubSetIndex];
    int ballIndex = saveBytes[kSavedBallIndex];
    int caddieIndex = saveBytes[kSavedCaddieIndex];
    this->characterInventoryIndex = characterIndex;
    this->caddieInventoryIndex = caddieIndex;
    this->clubSetInventoryIndex = clubSetIndex;
    this->ballInventoryIndex = ballIndex;
    this->flags = previewFlags;
    this->levelIndex = PlayerRank_FindTierForValue(((SavedPreviewSource*)saveRecord)->rankValue);
    this->caddieRankBand = 0;
    if (this->caddieInventoryIndex >= 0 && this->caddieInventoryIndex < kCharacterCount)
        this->caddieRankBand = PlayerRank_FindThresholdBand(
            ((SavedPreviewSource*)saveRecord)->characterValues[this->caddieInventoryIndex].value);
    int itemIndex0 = saveBytes[kSavedFirstItemIndex], itemIndex1 = saveBytes[kSavedFirstItemIndex + 1];
    int itemIndex2 = saveBytes[kSavedFirstItemIndex + 2], itemIndex3 = saveBytes[kSavedFirstItemIndex + 3];
    int itemIndex4 = saveBytes[kSavedFirstItemIndex + 4], itemIndex5 = saveBytes[kSavedFirstItemIndex + 5];
    int itemIndex6 = saveBytes[kSavedFirstItemIndex + 6], itemIndex7 = saveBytes[kSavedFirstItemIndex + 7];
    this->equippedItemInventoryIndex0 = itemIndex0;
    this->equippedItemInventoryIndex1 = itemIndex1;
    this->equippedItemInventoryIndex2 = itemIndex2;
    this->equippedItemInventoryIndex3 = itemIndex3;
    this->equippedItemInventoryIndex4 = itemIndex4;
    this->equippedItemInventoryIndex5 = itemIndex5;
    this->equippedItemInventoryIndex6 = itemIndex6;
    this->equippedItemInventoryIndex7 = itemIndex7;
    // Flatten the three groups of eight outfit parts, then hair and the auxiliary
    // part. These shorts are part-inventory indices, resolved later. The fields
    // below include the fixed save-record prefix; part groups keep their stored
    // order, and their equipment categories are not named here.
    char* entry = (char*)saveRecord + characterIndex * kSavedOutfitStride;
    for (int group = 0; group < kOutfitGroupCount; group++) {
        for (int part = 0; part < kPartsPerOutfitGroup; part++) {
            *(short*)(((char*)&this->partInventoryIndices) + group * kPartGroupBytes + part * 2) =
                *(short*)(entry + kSavedOutfitParts + group * kPartGroupBytes + part * 2);
        }
    }
    this->hairStyleIndex = (signed char)entry[kSavedOutfitHair];
    this->baseTextureVariant = 0;
    this->caddieItemInventoryIndex = saveBytes[kSavedCaddieItemIndex];
    this->auxiliaryPartInventoryIndex = entry[kSavedOutfitAuxiliary];
    for (int i = 0; i < 5; i++)
        *(((char*)&this->characterStatLevels) + i) = 0;
    ClubCustomization_GetFivePartValues((char*)this + kClubCustomizationOffset, saveRecord, 0);
}

struct VDestroy {
    virtual void destroy(int);
};

// Apply changed animation names and advance every existing preview avatar.
int CCharacterPreviewTable::Update(void* base, float dt) {
    char* recordCursor = (char*)base;
    int i = 0;
    while (i < AsCharacterPreviewTable(base)->count) {
        if (AsCharacterPreviewRecord(recordCursor)->renderObject) {
            if (((AsCharacterPreviewRecord(recordCursor)->flags) >> 14) & 1) {
                void* name;
                if (AsCharacterPreviewRecord(recordCursor)->animationName[0] >> 31)
                    name = (*(void**)&AsCharacterPreviewRecord(recordCursor)->animationName[2]);
                else
                    name = (char*)&AsCharacterPreviewRecord(recordCursor)->animationName + 1;
                if (CAsyncExhibition::PlayFixedNameAnimation(AsCharacterPreviewRecord(recordCursor)->renderObject,
                                                             name))
                    AsCharacterPreviewRecord(recordCursor)->flags &= ~kPreviewNameChanged;
            }
            struct VUpdate {
                virtual void a() DECOMP_LOCAL_VIRTUAL;
                virtual void update(float) DECOMP_LOCAL_VIRTUAL;
            };
            ((VUpdate*)AsCharacterPreviewRecord(recordCursor)->renderObject)->update(dt);
        }
        recordCursor += kCharacterPreviewRecordSize;
        i++;
    }
    return 1;
}

// Advance the shared context's preview avatars and retry pending animation-name changes.
int RenderContext_UpdateCharacterPreviews(float dt) {
    char* base;
    int i = 0;
    char* recordCursor;
    base = (char*)RenderContext_GetOrCreate() + kRenderPlayersOffset;
    recordCursor = base;
    while (i < AsCharacterPreviewTable(base)->count) {
        if (AsCharacterPreviewRecord(recordCursor)->renderObject) {
            if (((AsCharacterPreviewRecord(recordCursor)->flags) >> 14) & 1) {
                void* name;
                if (AsCharacterPreviewRecord(recordCursor)->animationName[0] >> 31)
                    name = (*(void**)&AsCharacterPreviewRecord(recordCursor)->animationName[2]);
                else
                    name = (char*)&AsCharacterPreviewRecord(recordCursor)->animationName + 1;
                if (CAsyncExhibition::PlayFixedNameAnimation(AsCharacterPreviewRecord(recordCursor)->renderObject,
                                                             name))
                    AsCharacterPreviewRecord(recordCursor)->flags &= ~kPreviewNameChanged;
            }
            struct VUpdate {
                virtual void a() DECOMP_LOCAL_VIRTUAL;
                virtual void update(float) DECOMP_LOCAL_VIRTUAL;
            };
            ((VUpdate*)AsCharacterPreviewRecord(recordCursor)->renderObject)->update(dt);
        }
        recordCursor += kCharacterPreviewRecordSize;
        i++;
    }
    return 1;
}

// Translate saved character/hair/part indices into an avatar equipment selection.
int CCharacterPreviewRecord::BuildItemSelection(void* descriptor, void* dest) {
    if (!dest)
        return 0;
    void* id = ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                              AsCharacterPreviewRecord(descriptor)->characterInventoryIndex);
    void* found = ItemInventory_GetHairStyleIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                 AsCharacterPreviewRecord(descriptor)->hairStyleIndex);
    int mode = 0;
    if (found)
        mode = ((IFF_STRUCT::sHairStyle*)ItemInventory_FindHairStyleData(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                                         (int)found))
                   ->colourIndex;
    unsigned int rawTextureVariant = AsCharacterPreviewRecord(descriptor)->baseTextureVariant;
    unsigned char textureVariant = rawTextureVariant;
    void* values[24];
    char* dp = (char*)descriptor;
    for (unsigned int i = 0; i < kOutfitPartCount; i++) {
        values[i] = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                              *(short*)(dp + kPreviewPartInventoryIndicesOffset));
        dp = dp + 2;
    }
    ItemSelection_BuildCharacterOutfit(dest, id, mode, textureVariant, values, 0xff);
    return 1;
}

// Destroy each avatar renderer while preserving the table's record storage.
int CCharacterPreviewTable::DestroyRenderObjects(void* base) {
    char* recordCursor = (char*)base;
    int i = 0;
    while (i < AsCharacterPreviewTable(base)->count) {
        void* obj = AsCharacterPreviewRecord(recordCursor)->renderObject;
        // The inner `if (obj)` repeats the outer test (inlined delete-guard); it is: delete the avatar if present.
        if (obj) {
            if (obj)
                ((VDestroy*)obj)->destroy(1);
            AsCharacterPreviewRecord(recordCursor)->renderObject = 0;
        }
        recordCursor += kCharacterPreviewRecordSize;
        i++;
    }
    return 1;
}

#pragma opt_common_subs off
// Destroy the avatar and its owned animation-name string; optionally delete the record.
// The name is a tagged string: bit 31 of word 0 set = heap text pointer at name[2] (freed here), else inline.
// The repeated `if (child)` / `field != 0` tests are inlined-destructor guards and are always consistent.
void* PlayerRenderRecord_dtor(void* obj, int shouldDelete) {
    if (obj) {
        void* child = AsCharacterPreviewRecord(obj)->renderObject;
        if (child) {
            if (child)
                ((VDestroy*)child)->destroy(1);
            AsCharacterPreviewRecord(obj)->renderObject = 0;
        }
        void* field = ((char*)&AsCharacterPreviewRecord(obj)->animationName);
        if (field != 0) {
            if ((AsCharacterPreviewRecord(obj)->animationName[0] >> 31) != 0)
                appFreeUni__FPv((*(void**)&AsCharacterPreviewRecord(obj)->animationName[2]));
        }
        if (shouldDelete > 0)
            __dl__FPv(obj);
    }
    return obj;
}
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
// Destroy table avatars and the four constructed records; optionally delete the table.
// Plain C: for each of `count` records delete its avatar; run the per-record destructor over all four
// records (array of kCharacterPreviewRecordSize bytes); free the table if shouldDelete > 0.
void* CharacterPreviewTable_dtor(void* obj, int shouldDelete) {
    if (obj) {
        int i = 0;
        char* recordCursor = (char*)obj;
        while (i < AsCharacterPreviewTable(obj)->count) {
            void* child = AsCharacterPreviewRecord(recordCursor)->renderObject;
            if (child) {
                if (child)
                    ((VDestroy*)child)->destroy(1);
                AsCharacterPreviewRecord(recordCursor)->renderObject = 0;
            }
            recordCursor += kCharacterPreviewRecordSize;
            i++;
        }
        __destroy_arr(obj, (void*)PlayerRenderRecord_dtor, kCharacterPreviewRecordSize, kPlayerCount);
        if (shouldDelete > 0)
            __dl__FPv(obj);
    }
    return obj;
}
#pragma use_lmw_stmw reset

// Initialize the enemy-data interface and clear its shared-file handle.
void CPangyaEnemyData::Init() {
    (*(int*)&dataBuffer) = 0;
    enemyVtable = (void*)__vt__16CPangyaEnemyData;
    *(int*)&sharedFile = 0;
}

// Return one of the four preview records, or null for an invalid index.
void* CCharacterPreviewTable::GetRecordByIndex(void* base, int idx) {
    if (idx < 0 || idx >= kPlayerCount)
        return 0;
    return (char*)base + idx * kCharacterPreviewRecordSize;
}

// Validate a record index and create or update its avatar preview.
int CCharacterPreviewTable::SetupRecordAvatar(void* base, int idx, void* viewport, int suppliedSelection, float scale) {
    if (idx < 0 || AsCharacterPreviewTable(base)->count <= idx)
        return 0;
    return ((CCharacterPreviewRecord*)((char*)base + idx * kCharacterPreviewRecordSize))
        ->SetupAvatar(viewport, suppliedSelection, scale);
}

// Copy a descriptor while retaining the destination's live avatar renderer.
void* CCharacterPreviewRecord::CopyPreservingRenderer(void* obj, void* src) {
    // This is a shallow byte copy, including tagged name storage; it does not
    // duplicate heap strings. Source/destination name lifetime must already be
    // managed by the caller. Only the destination renderer is explicitly kept.
    void* saved = AsCharacterPreviewRecord(obj)->renderObject;
    Memory_Copy(obj, src, kCharacterPreviewRecordSize);
    AsCharacterPreviewRecord(obj)->renderObject = saved;
    return obj;
}

// Open the enemy-data resource and retain its loaded-buffer pointer.
void CPangyaEnemyData::LoadSharedFile() {
    void* sharedFileHandle = SharedFile_Create(s_enemyDataResourcePath, 1);
    sharedFile = sharedFileHandle;
    dataBuffer = CSharedFilePtr_GetLoadedBuffer(sharedFileHandle);
}

// Draw every instantiated avatar in the table.
void CCharacterPreviewTable::DrawAvatars(void* base) {
    char* recordCursor = (char*)base;
    int i = 0;
    while (i < AsCharacterPreviewTable(base)->count) {
        void* avatar = AsCharacterPreviewRecord(recordCursor)->renderObject;
        if (avatar) {
            struct VObj {
                virtual void a() DECOMP_LOCAL_VIRTUAL;
                virtual void b() DECOMP_LOCAL_VIRTUAL;
                virtual void c() DECOMP_LOCAL_VIRTUAL;
            };
            ((VObj*)avatar)->c();
        }
        recordCursor = recordCursor + kCharacterPreviewRecordSize;
        i = i + 1;
    }
}

// Enable or disable scripted updates on every instantiated avatar.
void CCharacterPreviewTable::SetScriptEnabled(void* base, int arg) {
    char* recordCursor = (char*)base;
    int i = 0;
    while (i < AsCharacterPreviewTable(base)->count) {
        void* obj = AsCharacterPreviewRecord(recordCursor)->renderObject;
        if (obj)
            ((CAsyncExhibition*)obj)->SetScriptEnabled(arg);
        recordCursor += kCharacterPreviewRecordSize;
        i++;
    }
}

// Release an owned pointer-vector buffer and optionally delete the vector object.
void* dtor_80247200(void* obj, int shouldDelete) {
    if (obj) {
        *(int*)((char*)obj + 4) = 0;
        if (obj) {
            if (*(void**)obj) {
                *(int*)((char*)obj + 4) = 0;
                appFreeUni__FPv(*(void**)obj);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(obj);
    }
    return obj;
}

// Release the enemy-data shared-file handle and optionally delete the object.
CPangyaEnemyData::~CPangyaEnemyData() {
    void* p = sharedFile;
    enemyVtable = __vt__16CPangyaEnemyData;
    if (p) {
        CSharedFileHandle::Destroy(p, 1);
        *(int*)&sharedFile = 0;
    }
}

// Resolve equipment and character framing, then create or refresh a scaled avatar preview.
int CCharacterPreviewRecord::SetupAvatar(void* viewport, int suppliedSelection, float scale) {
    // viewport is a borrowed int[4] {left, top, right, bottom} in screen pixels.
    // suppliedSelection is a borrowed ItemSelection address in an int-sized slot;
    // zero builds stack equipment instead. Validation may refresh that selection.
    // scale is multiplied by a per-character framing factor.
    // Return 1 after equipment application. Returning 0 may leave a newly owned
    // renderer installed; PlayerRenderRecord_dtor releases it.
    double localSelection[13];
    WVector rotation;
    int adjustedViewport[4];
    float viewportOffset[2];
    // The ABI supplies an optional ItemSelection address through this integer-sized parameter.
    void* equipment = (void*)suppliedSelection;
    ItemSelection_Init(localSelection);
    if (!suppliedSelection) {
        CCharacterPreviewRecord::BuildItemSelection(this, localSelection);
        equipment = localSelection;
    }
    if (!ItemSelection_Validate(equipment))
        ItemSelection_RefreshPrimaryItems(equipment);
    int characterIndex = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, *(int*)equipment);
    float characterScale;
    int validCharacter;
    if (characterIndex < 0 || characterIndex >= 8) {
        validCharacter = 0;
    } else {
        characterScale = kCharacterPreviewScales[characterIndex];
        validCharacter = 1;
    }
    if (!validCharacter)
        return 0;
    if (characterIndex < 0 || characterIndex >= 8) {
        validCharacter = 0;
    } else {
        rotation = *(WVector*)((char*)&s_characterPreviewRotations + characterIndex * 0xc);
        validCharacter = 1;
    }
    if (!validCharacter)
        return 0;
    if (characterIndex < 0 || characterIndex >= 8) {
        validCharacter = 0;
    } else {
        Vec2_Copy(viewportOffset, (char*)&s_characterPreviewViewportOffsets + characterIndex * 8);
        validCharacter = 1;
    }
    if (!validCharacter)
        return 0;
    int viewportLeft = *(int*)viewport;
    scale *= characterScale;
    int viewportTop = *((int*)viewport + 1);
    int viewportRight = *((int*)viewport + 2);
    int viewportBottom = *((int*)viewport + 3);
    // viewportOffset is {dx, dy}, applied to both rectangle corners. Each
    // translated corner is truncated to int independently. Rotation is passed
    // through in XYZ radians; the exhibition setter does the unit handling.
    adjustedViewport[0] = (int)((float)viewportLeft + viewportOffset[0]);
    adjustedViewport[1] = (int)((float)viewportTop + viewportOffset[1]);
    adjustedViewport[2] = (int)((float)viewportRight + viewportOffset[0]);
    adjustedViewport[3] = (int)((float)viewportBottom + viewportOffset[1]);
    void** rendererSlot = &this->renderObject;
    // Create and own a missing renderer, clearing its animation name; otherwise
    // update the existing renderer's viewport without replacing it.
    if (!*rendererSlot) {
        void* obj = CAsyncExhibitionAvatar::Create(adjustedViewport, 0, scale);
        unsigned int nameStorage = this->animationName[0];
        unsigned short previewFlags = this->flags;
        int nameLength = (*(unsigned char*)&this->animationName) & 0x7f;
        if (nameStorage >> 31)
            nameLength = (*(int*)&this->animationName[1]);
        char* name = charaSelectStrings + kDefaultAnimationNameOffset;
        this->flags = previewFlags & ~kPreviewNameChanged;
        this->renderObject = obj;
        char temp = 0;
        String_ReplaceRange(this->animationName, 0, nameLength, name, name + strlen(name), &temp);
        if (!this->renderObject)
            return 0;
    } else {
        ((CAsyncExhibition*)(*rendererSlot))->SetViewport(adjustedViewport, scale);
    }
    ((CAsyncExhibition*)(this->renderObject))->SetRotation(&rotation);
    if (!((CAsyncExhibitionAvatar*)(this->renderObject))->SetEquipment(equipment, 0))
        return 0;
    return 1;
}
struct HoverList {
    int* ptr;
    volatile int count;
    int cap;
};

struct GateByte {
    CW_BITS3(unsigned char b7 : 1, signed char b6 : 1, unsigned char rest : 6)
};
struct Flags16 {
    CW_BITS2(unsigned short top : 1, unsigned short lo : 15)
};

// Resolve per-player character hovers and rebuild previews when the hovered selection changes.
int CCharaSelect::UpdateHoveredAvatars() {
    // Required live self; layout may be null (return 0 unchanged). Failure to
    // obtain a context also returns 0. Otherwise return 1 means the pass ran,
    // not that every preview was built: the SetupAvatar result is ignored.
    // Index domains: player i=0..3; hover.ptr entries are layout element IDs;
    // candidate/liveCharacterIndex=0..7 or -1; history.back() is a SAVE RECORD
    // index 0..3 (four 0x828-byte records precede vectors at 0x20A0), and
    // renderSlotIndex=0..3 or -1. History length is checked, its contents are not.
    // lastHoveredCharacter must borrow four writable ints for this screen's life.
    // Render table count must stay 0..4. Do not replace these domains with one
    // common index: character number, player, save record and render slot differ.
    // Layout/context/preview records are borrowed. Layout_CollectHovered creates
    // the temporary owned hover vector; every normal skip/completion frees it.
    // Created avatar renderers belong to the context records, released by
    // PlayerRenderRecord_dtor/DestroyRenderObjects, never by this hover vector.
    // Missing gate, unchanged cache, missing slot or live-map miss ends this
    // player's scan. No recognized ID caches -1. A recognized candidate is
    // cached EVEN WHEN a later check or SetupAvatar fails, so identical hover
    // need not retry a failed preview until the cached character changes.
    // Setup may leave renderer/viewport/name/equipment partly changed on failure;
    // loading-indicator placement still runs if a renderer exists. No rollback.
    // Per player: require a nonempty selection-history vector; inspect hovered
    // layout IDs in collection order; map through the eight-entry character map;
    // test the saved character gate; rebuild at most one preview; cache the candidate.
    // The map is character index -> layout ID, not eight consecutive UI IDs.
    // Repeated singleton blocks preserve lazy construction at each access site.
    // Each temporary hover array is owned here and freed on skip or completion.
    if (!layout)
        return 0;
    if (!RenderContext_GetOrCreate())
        return 0;
    char* loadingIndicatorPositions = kCharacterPreviewLoadingIndicatorPositions;
    int* liveCharacterElementIds = kCharacterPreviewLayoutElementIds;
    struct Zeros {
        int a, b, c;
    } emptyTableCounts;
    emptyTableCounts.a = 0;
    emptyTableCounts.b = 0;
    emptyTableCounts.c = 0;
    char* base = s_renderContext;
    char* base2 = s_renderContext;
    float previewScale = kHoveredCharacterPreviewScale;
    for (int i = 0; i < kPlayerCount; i++) {
        HoverList hover;
        hover.ptr = 0;
        hover.count = 0;
        hover.cap = 0;
        // A. Resolve the context for the history-empty test. The construction
        // block ends at publication to Sing_805DC648(); hover selection resumes below.
        if (((int&)Sing_805DC648()) == 0) {
            if (s_renderContextInitialized == 0) {
                char* playerRecordsEnd = base + kRenderVectorsOffset;
                char* playerRecordCursor = base;
                do {
                    SaveData_InitPlayerRecord(playerRecordCursor);
                    playerRecordCursor += kPlayerRecordSize;
                } while (playerRecordCursor < playerRecordsEnd);
                ((IntVectorArray*)(playerRecordsEnd))->__ct();
                char* table = base + kRenderPlayersOffset;
                __construct_array(table, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                                  kCharacterPreviewRecordSize, kPlayerCount);
                AsCharacterPreviewTable(table)->count = *(volatile int*)&emptyTableCounts.a;
                InitializeNewRenderContext(base);
                __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                         s_renderContextRegistrations_805041E8);
                s_renderContextInitialized = 1;
            }
            ((int&)Sing_805DC648()) = (int)base;
        }
        if (*(int*)((char*)((int&)Sing_805DC648()) + i * 0xc + 0x20a4) == 0) {
            if (hover.ptr) {
                hover.count -= hover.count;
                appFreeUni__FPv(hover.ptr);
            }
            continue;
        }
        Layout_CollectHovered(layout, &hover, i);
        // Phase 1: snapshot element IDs. candidateCharacter records the latest recognized
        // candidate even if its gate later blocks creation (it is still cached).
        CharacterElementMap m = CopyCharacterElementMap(liveCharacterElementIds);
        int candidateCharacter = -1;
        int* end = hover.ptr + hover.count;
        for (int* p = hover.ptr; p != end; p++) {
            if (FindCharacterElementIndex(m, *p) == -1)
                continue;
            candidateCharacter = FindCharacterElementIndex(m, *p);
            // B. Resolve context again after layout collection; then read the
            // selected save-record index from this player's history.back().
            if (((int&)Sing_805DC648()) == 0) {
                if (s_renderContextInitialized == 0) {
                    char* playerRecordsEnd = base + kRenderVectorsOffset;
                    char* playerRecordCursor = base;
                    do {
                        SaveData_InitPlayerRecord(playerRecordCursor);
                        playerRecordCursor += kPlayerRecordSize;
                    } while (playerRecordCursor < playerRecordsEnd);
                    ((IntVectorArray*)(playerRecordsEnd))->__ct();
                    char* table = base + kRenderPlayersOffset;
                    __construct_array(table, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                                      kCharacterPreviewRecordSize, kPlayerCount);
                    AsCharacterPreviewTable(table)->count = *(volatile int*)&emptyTableCounts.b;
                    InitializeNewRenderContext(base);
                    __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                             s_renderContextRegistrations_805041E8);
                    s_renderContextInitialized = 1;
                }
                ((int&)Sing_805DC648()) = (int)base;
            }
            int historyCount = *(int*)((char*)((int&)Sing_805DC648()) + i * 0xc + 0x20a4);
            int* historyRecordIndices = *(int**)((char*)((int&)Sing_805DC648()) + i * 0xc + kRenderVectorsOffset);
            int selectedSaveRecord = historyRecordIndices[historyCount - 1];
            // C. Resolve context for the outfit-gate test. Do not hoist across
            // calls above: lazy construction and publication occur at each site.
            if (((int&)Sing_805DC648()) == 0) {
                if (s_renderContextInitialized == 0) {
                    char* playerRecordsEnd = base + kRenderVectorsOffset;
                    char* playerRecordCursor = base2;
                    do {
                        SaveData_InitPlayerRecord(playerRecordCursor);
                        playerRecordCursor += kPlayerRecordSize;
                    } while (playerRecordCursor < playerRecordsEnd);
                    ((IntVectorArray*)(playerRecordsEnd))->__ct();
                    char* table = base + kRenderPlayersOffset;
                    __construct_array(table, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                                      kCharacterPreviewRecordSize, kPlayerCount);
                    AsCharacterPreviewTable(table)->count = *(volatile int*)&emptyTableCounts.c;
                    InitializeNewRenderContext(base2);
                    __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                             s_renderContextRegistrations_805041E8);
                    s_renderContextInitialized = 1;
                }
                ((int&)Sing_805DC648()) = (int)base;
            }
            GateByte* outfitGate = (GateByte*)((char*)((int&)Sing_805DC648()) + selectedSaveRecord * kPlayerRecordSize +
                                               candidateCharacter * kSavedOutfitStride + 0x2cc);
            // Phase 2: bit 6 of the outfit's first byte must be nonzero. Its
            // broader meaning is unresolved. Failure stops this player's scan;
            // it does not try another hovered character. The second map read
            // uses live IDs; retain the distinction from the earlier snapshot.
            if (outfitGate->b6 == 0)
                break;
            // D. Read the live UI-ID map once more before comparing the cache.
            // The earlier snapshot validates candidates; this scan resolves the
            // selection passed to UpdateSlotRenderRecord. Preserve both scans.
            int m0 = liveCharacterElementIds[0];
            int m1 = liveCharacterElementIds[1];
            int m2 = liveCharacterElementIds[2];
            int m3 = liveCharacterElementIds[3];
            int m4 = liveCharacterElementIds[4];
            int m5 = liveCharacterElementIds[5];
            int m6 = liveCharacterElementIds[6];
            int m7 = liveCharacterElementIds[7];
            int id = *p;
            int liveCharacterIndex;
            if (id == m0)
                liveCharacterIndex = 0;
            else if (id == m1)
                liveCharacterIndex = 1;
            else if (id == m2)
                liveCharacterIndex = 2;
            else if (id == m3)
                liveCharacterIndex = 3;
            else if (id == m4)
                liveCharacterIndex = 4;
            else if (id == m5)
                liveCharacterIndex = 5;
            else if (id == m6)
                liveCharacterIndex = 6;
            else if (id == m7)
                liveCharacterIndex = 7;
            else
                liveCharacterIndex = -1;
            if (liveCharacterIndex == -1)
                break;
            if (liveCharacterIndex == *(int*)((char*)((CharaHoverCache*)this)->lastHoveredCharacter + i * 4))
                break;
            int renderSlotIndex = (int)RenderContext_GetLastSlotRecordIndex(RenderContext_GetOrCreate(), i);
            if (renderSlotIndex == -1)
                break;
            void* info = RenderContext_GetCurrentSlotRenderRecord(RenderContext_GetOrCreate(), i);
            bool nz;
            if (info)
                nz = ((Flags16*)((char*)info + kPreviewFlagsOffset))->top;
            else
                nz = false;
            CharaSelect_UpdateSlotRenderRecord(this, i, liveCharacterIndex, nz, renderSlotIndex, 0);
            // Phase 3: refresh the slot descriptor and its 150-pixel viewport;
            // place the loading indicator using its separate two-value table.
            int rect[4];
            rect[0] = renderSlotIndex * kPreviewWidth + kPreviewLeft;
            rect[1] = kPreviewTop;
            rect[2] = rect[0] + kPreviewWidth;
            rect[3] = kPreviewBottom;
            char* table = (char*)RenderContext_GetOrCreate() + kRenderPlayersOffset;
            if (renderSlotIndex >= 0 && AsCharacterPreviewTable(table)->count > renderSlotIndex) {
                ((CCharacterPreviewRecord*)(table + renderSlotIndex * kCharacterPreviewRecordSize))
                    ->SetupAvatar(rect, 0, previewScale);
            }
            int off = renderSlotIndex * kCharacterPreviewRecordSize;
            if (*(void**)((char*)RenderContext_GetOrCreate() + off + kRenderAvatarOffset) != 0) {
                void* o = *(void**)((char*)RenderContext_GetOrCreate() + off + kRenderAvatarOffset);
                ((CAsyncExhibition*)o)
                    ->SetLoadingIndicatorPosition((const float*)(loadingIndicatorPositions + renderSlotIndex * 8));
            }
            break;
        }
        // Cache the recognized candidate even when a gate/cache/slot check broke
        // the scan. -1 records that no recognized character element was hovered.
        *(int*)((char*)((CharaHoverCache*)this)->lastHoveredCharacter + i * 4) = candidateCharacter;
        if (hover.ptr) {
            hover.count -= hover.count;
            appFreeUni__FPv(hover.ptr);
        }
    }
    return 1;
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622B8", _ctors_u80245F84_0, InitCharacterPreviewTransforms);
#pragma pop

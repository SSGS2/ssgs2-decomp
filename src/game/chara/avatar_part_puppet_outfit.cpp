// Avatar outfits: validating and applying item selections (hair, base, parts), attachment slots,
// part hide masks and texture variants. Also the puppet load request for an avatar, the petal
// particle emitter, and the string and vector helpers.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
struct AvatarPartCountFields {
    char reserved[0x30];
    int partCount;
};
struct AvatarPartBindingFields {
    char reserved[0x1C4];
    void* resource;
    void* part;
};
struct AvatarPartPuppetFields {
    char reserved[0x38];
    void* puppet;
};

// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum PetModelTextOffset {
    kPetTextVectorLengthError = 0xac,    // vector length error
    kPetTextLengthErrorType = 0xc0,      // std::length_error type chain
    kPetTextBitmapVariantFormat = 0x103, // %02d.gsr
    kPetTextReplaceComplete = 0x10c,     // APET REPLACE OK
};

// Item IDs use the Wii IFF layout documented in docs/iff-formats.md.
enum OutfitItemId {
    kItemKindPayloadMask = 0x03FFFFFF,
    kPartKind = 0x08000000,
    kDefaultPart = 0x400,
    kDefaultSubPart = 0x200,
    kDefaultPartId = kPartKind | kDefaultPart,
    kPartCharacterShift = 18,
    kPartSlotShift = 13,
    kPartSlotStride = 0x2000,
    kPartSlotMask = 0x1F,
    kPartCharacterMask = 0xFF,
    kOutfitItemBytes = 0x60,
    kAvatarAttachmentCount = 5
};

// Resource request prefix shared by the replace/poll/bounds functions.
struct PuppetLoadFields {
    unsigned int loadState;   // 0x00: 1 while awaiting a puppet resource
    unsigned int characterId; // 0x04: IFF character type ID
    void* puppet;             // 0x08
    void* hairBitmap;         // 0x0C
    void* partBitmap;         // 0x10
    void* baseBitmap;         // 0x14
    char reserved18[4];
    unsigned int* animationResources; // 0x1C
    unsigned int animationCount;      // 0x20
    unsigned int animationCapacity;   // 0x24
    void* loadedResource;             // 0x28
};
enum PuppetResourceVectorLimits {
    kMaxAnimationHandles = 0x3FFFFFFF,
    kSmallAnimationVectorLimit = 0x15555555,
    kMediumAnimationVectorLimit = 0x2AAAAAAA,
    kPuppetLoadIdle = 0,
    kPuppetLoadPending = 1
};
// Word aliases retain the original unsigned-int aliasing during the bounds pass.
// Derive indices from the typed layout so field/stride changes have a single source.
enum PuppetLoadWord {
    kLoadStateWord = (int)&((PuppetLoadFields*)0)->loadState / 4,
    kLoadPuppetWord = (int)&((PuppetLoadFields*)0)->puppet / 4,
    kLoadAnimationsWord = (int)&((PuppetLoadFields*)0)->animationResources / 4,
    kLoadAnimationCountWord = (int)&((PuppetLoadFields*)0)->animationCount / 4,
    kLoadCompletedResourceWord = (int)&((PuppetLoadFields*)0)->loadedResource / 4
};

// These are views of the existing avatar storage, not new runtime objects.
struct AvatarAppearanceFields {
    char reserved[0x1B8];
    unsigned char partsReady;
    char reserved1B9[3];
    unsigned int characterId;
    unsigned char primaryCount, totalCount, hairVariant, baseVariant;
};
struct AvatarAttachmentFields {
    char reserved[0x284];
    void* attachments[kAvatarAttachmentCount];
};

#include "iff/IffStruct.h"
#include "classes/ItemSelection801B7688.h"
#include "classes/MWStringStorage.h"
#include "classes/CSharedDoc.h"
#include "classes/PetalSilicaEmitterView.h"
#include <decomp/cw_bitfields.h>
#define AsItemSelection801B7688(p) ((ItemSelection801B7688*)(p))

// Byte 6 chooses hair/part textures; byte 7 chooses the character base bitmap.
static inline unsigned char& OutfitHairVariant(void* selection) {
    return AsItemSelection801B7688(selection)->textureVariant;
}
// Borrow byte 7, the character base-bitmap variant; no resource is loaded here.
static inline unsigned char& OutfitBaseVariant(void* selection) {
    return AsItemSelection801B7688(selection)->bitmapVariant;
}

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char petModelResourceStrings[];
}
}

// Select the larger borrowed value, keeping the original reference-return ABI.
template <class T> static inline const T& max_ref(const T& a, const T& b) {
    return a < b ? b : a;
}

extern "C" {
extern void Vec3_Zero(void*);
extern void* __ml__FfRC7WVector(void*, void*, float);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void WPuppet_SetBoneVisibilityRecursive(void*, int, int, int);
extern void* ntPetal_Ctor(void*, void*);
extern char __vt__7ntPetal[];
extern const float kFloat_1_PetEffects;
extern float kFloat_0_7_PetEffects;
extern float kFloat_Neg112_PetParticleExtent;
extern const float kFloat_0_3_PetEffects;
extern const float kFloat_112_PetParticleExtent;
extern void* g_itemSelectionInventory;
extern int AvatarModel_ReloadPartResources(void*, void*);
extern void* ItemSelection_CopyValidatedChanges(void*, void*, unsigned int);
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern int ResourceManager_RequestModelLoad(void*, void*);
extern int AvatarModel_ApplyTextureVariant(void*, void*, unsigned char);
extern int CharacterModel_ApplyBitmapVariant(void*, void*, unsigned char);
extern void* ItemInventory_FindPartData(void*, unsigned int);
extern void AvatarModel_ApplyPartHideMasks(void*, int);
extern void __dl__FPv(void*);
extern const float kFloat_6_2831855;
extern float s_twoPi_801B6670;
extern float s_avatarAttachmentFullTurnRadians; // Writable full-turn constant initialized by the avatar-model unit.
extern int ItemSelection_BuildCharacterOutfit(void*, unsigned int, unsigned int, unsigned int, void*, int);
extern "C" unsigned int strlen(const char*);
extern "C" void* memset(void*, int, unsigned int);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern void OSReport(const char*, ...);
extern int String_FindLastOf(void*, const char*, int, unsigned int);
extern void* g_resourceManager;
extern void* g_sharedDoc;
extern void* ItemInventory_FindCharacterData(void*, unsigned int);
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern char* strrchr(char*, int);
extern void WPuppet_ApplyNamedResourcePairRecursive(void*, void*, void*, int);
extern void ResourceManager_ReleaseId(void*, void*);
extern void WPuppet_UpdateBoundsFromResourceBox(void*, int, int);
extern void WPuppet_AccumulateAnimationBounds(void*, int, unsigned int, int, int, int);
extern unsigned int s_characterVariantPartSlots[];
extern char petModelResourceStrings[];
extern int String_ParseDecimalInt(char*);
extern int sprintf(char*, const char*, ...);
extern char* strcpy(char*, const char*);
extern int ResourceManager_PollResourceRead(void*);
extern void* ResourceManager_PollPuppetResourceLoad(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __throw(void*, void*, void*) __attribute__((noreturn));
extern char __dt__Q23std12length_errorFv[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

// Apply validated outfit edits, refresh textures, and update hidden model parts.
void* AvatarModel_ApplyValidatedItemChanges(void* avatar, void* selection, unsigned int validationFlags) {
    if (!selection)
        return 0;
    void* changes =
        ItemSelection_CopyValidatedChanges(&((AvatarAppearanceFields*)avatar)->characterId, selection, validationFlags);
    if (changes) {
        AvatarModel_ReloadPartResources(avatar, changes);
        AvatarModel_ApplyTextureVariant(avatar, selection, ((unsigned char*)selection)[6]);
        CharacterModel_ApplyBitmapVariant(avatar, 0, ((unsigned char*)selection)[7]);
        ((AvatarAppearanceFields*)avatar)->partsReady = 1;
        AvatarModel_ApplyPartHideMasks(avatar, 1);
        return changes;
    } else {
        return changes;
    }
}

// Release previous animation resources and start a replacement puppet load request.
int PlayerModelLoad_ReplacePendingRequest(void* request, void* resourceName) {
    for (unsigned int* animation = ((PuppetLoadFields*)request)->animationResources;
         animation != ((PuppetLoadFields*)request)->animationCount + ((PuppetLoadFields*)request)->animationResources;
         animation++) {
        if (*animation)
            ResourceManager_ReleaseObject(g_resourceManager, (void*)*animation, 0);
    }
    ((PuppetLoadFields*)request)->loadedResource = 0;
    ((PuppetLoadFields*)request)->animationCount =
        ((PuppetLoadFields*)request)->animationCount - (int)((PuppetLoadFields*)request)->animationCount;
    ResourceManager_RequestModelLoad(g_resourceManager, resourceName);
    *(unsigned int*)request = 1;
    return 1;
}

// Search backward through a tagged string for any character in the supplied character set.
int String_FindLastOf(void* string, const char* characters, int lastPosition, unsigned int characterCount) {
    const signed char* data;
    unsigned int size;
    if ((*(unsigned int*)string >> 31) == 0) {
        data = (const signed char*)((char*)string + 1);
        size = *(unsigned char*)string & 0x7F;
    } else {
        // String_FindLastOf: original heap string pointer +0x8 and characterCount +0x4.
        data = (const signed char*)AsMWStringStorage(string)->data;
        size = AsMWStringStorage(string)->length;
    }
    // String_FindLastOf: scan only nonempty strings and character sets.
    if (size && characterCount) {
        if ((unsigned int)lastPosition > size - 1)
            lastPosition = size - 1;
        const signed char* cursor = data + lastPosition + 1;
        for (; cursor > data;) {
            --cursor;
            const char* character = characters;
            for (unsigned int i = 0; i < characterCount; i++, character++) {
                if (cursor[0] == *character)
                    return (int)(cursor - data);
            }
        }
    }
    return -1;
}

// Return one without side effects; checked callers ignore it, so success semantics are unproven.
int AvatarModel_DefaultReturnOneHook() {
    return 1;
}

// Refresh visibility of model parts covered by the current outfit.
void AvatarModel_RefreshCoveredParts(void* avatar) {
    AvatarModel_ApplyPartHideMasks(avatar, 1);
}

// Assign one of the five avatar attachment slots, rejecting an out-of-range index.
int AvatarModel_SetAttachment(void* avatar, unsigned char attachmentIndex, void* attachment) {
    if (attachmentIndex >= kAvatarAttachmentCount) {
        return 0;
    }
    ((AvatarAttachmentFields*)avatar)->attachments[attachmentIndex] = attachment;
    return 1;
}

// Apply supplied part changes and variants, mark resources ready, and refresh covered parts.
int AvatarModel_ApplyItemChanges(void* avatar, void* selection, void* partChanges) {
    AvatarModel_ReloadPartResources(avatar, partChanges);
    AvatarModel_ApplyTextureVariant(avatar, selection, ((unsigned char*)selection)[6]);
    CharacterModel_ApplyBitmapVariant(avatar, 0, ((unsigned char*)selection)[7]);
    ((AvatarAppearanceFields*)avatar)->partsReady = 1;
    AvatarModel_ApplyPartHideMasks(avatar, 1);
    return 1;
}

// Test whether an attachment pointer is present in any avatar attachment slot.
int AvatarModel_ContainsAttachment(void* avatar, void* attachment) {
    struct S {
        char pad[0x284];
        void* entries[5];
    };
    S* attachments = (S*)avatar;
    for (int i = 0; i < 5; i++) {
        if (attachment == attachments->entries[i])
            return 1;
    }
    return 0;
}

// Copy the fixed-size appearance record into the avatar appearance storage.
// `selection` is an ItemSelection payload (see classes/ItemSelection801B7688.h), copied as one 200-byte block:
//   head = +0 character id; +4..+7 = primaryCount, totalCount, textureVariant, bitmapVariant (as in ItemSelection801B7688);
//   data = +8..+0xC7: the part-ID words (firstPartId at +8, then the remaining slots) and whatever follows
//   them; the exact split inside data is not established here. The struct copy is a plain memberwise copy.
int AvatarModel_CopyAppearanceRecord(void* avatar, void* selection) {
    struct Words {
        unsigned int w[48];
    };
    struct Entry {
        unsigned int head;
        unsigned char primaryCount;
        unsigned char totalCount;
        unsigned char textureVariant;
        unsigned char bitmapVariant;
        Words data;
    };
    *(Entry*)(&((AvatarAppearanceFields*)avatar)->characterId) = *(Entry*)selection;
    return 1;
}

// Clear character, part counts, texture variants, and all 24 part-ID slots.
void* ItemSelection_Init(void* selection) {
    *(unsigned int*)selection = 0;
    AsItemSelection801B7688(selection)->primaryCount = 0;
    AsItemSelection801B7688(selection)->totalCount = 0;
    OutfitHairVariant(selection) = 0;
    OutfitBaseVariant(selection) = 0;
    memset(&AsItemSelection801B7688(selection)->firstPartId, 0, 0x60);
    return selection;
}

#pragma push
#pragma use_lmw_stmw on
// Rebuild default part IDs for primary body slots that exist in the inventory.
void ItemSelection_RefreshPrimaryItems(void* selection) {
    unsigned int slotBits;
    char* partCursor;
    unsigned int partId;
    int slot;
    int zero;
    memset(&AsItemSelection801B7688(selection)->firstPartId, 0, 0x60);
    partCursor = (char*)selection;
    slot = 0;
    slotBits = 0;
    zero = 0;
    partId = 0;
    for (; slot < AsItemSelection801B7688(selection)->primaryCount; slot++) {
        partId = (*(unsigned int*)selection << kPartCharacterShift) | (slotBits | kDefaultPartId);
        if (ItemInventory_FindPartData(g_itemSelectionInventory, partId))
            *(unsigned int*)(partCursor + 8) = partId;
        else
            *(unsigned int*)(partCursor + 8) = (unsigned int)zero;
        slotBits += kPartSlotStride;
        partCursor += 4;
    }
}
#pragma use_lmw_stmw reset
#pragma pop

// Search backward using the length of a NUL-terminated character set.
int String_FindLastOfCStr(void* string, const char* characters, int lastPosition) {
    unsigned int characterCount = strlen(characters);
    return String_FindLastOf(string, characters, lastPosition, characterCount);
}

// Set the item inventory used to resolve character and outfit part records.
void ItemSelection_SetInventory(void* inventory) {
    g_itemSelectionInventory = inventory;
}

// Initialize the model subsystem angle constant to one full turn.
void AvatarModel_InitAngleConstant() {
    s_twoPi_801B6670 = kFloat_6_2831855;
}

// Initialize the petal subsystem angle constant to one full turn.
void PetalEffect_InitAngleConstant() {
    s_avatarAttachmentFullTurnRadians = kFloat_6_2831855;
}

// Optionally deallocate this trivial model helper object.
void* dtor_801B7D1C(void* a, int deleteFlag) {
    if (a != 0 && deleteFlag > 0) {
        __dl__FPv(a);
    }
    return a;
}

struct ItemRec7AD4 {
    unsigned int characterId;
    char reserved4[4];
    char partIds[kOutfitItemBytes];
    CW_BITS2(unsigned char hairVariant : 5, unsigned char baseVariant : 3)
};

// Decode packed hair and base variants and build an outfit from the saved character and part IDs.
int ItemSelection_BuildOutfitFromRecord(void* selection, ItemRec7AD4* record) {
    return ItemSelection_BuildCharacterOutfit(selection, record->characterId, record->hairVariant, record->baseVariant,
                                              record->partIds, 0);
}

#pragma opt_common_subs off
// Recompute puppet bounds from all requested animations, or use static bounds when none exist.
void UpdatePuppetBoundsForAnimationList(void* request) {
    unsigned int* animation = ((PuppetLoadFields*)request)->animationResources;
    if (animation == ((PuppetLoadFields*)request)->animationResources + ((PuppetLoadFields*)request)->animationCount) {
        WPuppet_UpdateBoundsFromResourceBox(((PuppetLoadFields*)request)->puppet, 0, 1);
    } else {
        WPuppet_AccumulateAnimationBounds(((PuppetLoadFields*)request)->puppet, 1, *animation, 1, 0, 0);
        animation++;
        while (animation !=
               ((PuppetLoadFields*)request)->animationResources + ((PuppetLoadFields*)request)->animationCount) {
            WPuppet_AccumulateAnimationBounds(((PuppetLoadFields*)request)->puppet, 0, *animation, 1, 0, 0);
            animation++;
        }
    }
}
#pragma opt_common_subs reset

#pragma push
#pragma use_lmw_stmw on
// Set a character and populate its available default primary parts from IFF data.
void ItemSelection_SetBaseItem(unsigned int* selection, unsigned int characterId) {
    IFF_STRUCT::sChar* characterRecord = (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(
        ((CSharedDoc*)g_sharedDoc)->itemInventory, characterId); // original +0xA04
    if (!characterRecord)
        return;
    *selection = characterId;
    AsItemSelection801B7688(selection)->primaryCount = characterRecord->defaultPartCount;
    AsItemSelection801B7688(selection)->totalCount =
        characterRecord->defaultPartCount + characterRecord->optionalPartCount;
    memset(&AsItemSelection801B7688(selection)->firstPartId, 0, 0x60);
    int slot = 0;
    unsigned int partId = 0;
    unsigned int slotBits = 0;
    char* partCursor = (char*)selection;
    int zero = 0;
    for (; slot < AsItemSelection801B7688(selection)->primaryCount; slot++) {
        partId = (*(unsigned int*)selection << kPartCharacterShift) | (slotBits | kDefaultPartId);
        if (ItemInventory_FindPartData(g_itemSelectionInventory, partId))
            AsItemSelection801B7688(partCursor)->firstPartId = partId;
        else
            AsItemSelection801B7688(partCursor)->firstPartId = (unsigned int)zero;
        slotBits += kPartSlotStride;
        partCursor += 4;
    }
}
#pragma use_lmw_stmw reset
#pragma pop

// Reject wrong-character, missing, or overlapping parts and require coverage of every primary body slot.
int ItemSelection_Validate(void* selection) {
    unsigned int* partCursor = (unsigned int*)selection;
    unsigned int coveredSlots = 0;
    for (int slot = 0; slot < AsItemSelection801B7688(selection)->totalCount; partCursor++, slot++) {
        unsigned int partId = partCursor[2];
        if (!partId)
            continue;
        if ((*(unsigned int*)selection & kItemKindPayloadMask) !=
            ((partId >> kPartCharacterShift) & kPartCharacterMask))
            return 0;
        IFF_STRUCT::sPart* partRecord =
            (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_itemSelectionInventory, partId);
        if (!partRecord)
            return 0;
        unsigned int partSlots = partRecord->hideMask;
        if (coveredSlots & partSlots)
            return 0;
        coveredSlots |= partSlots;
    }
    unsigned int primaryCount = AsItemSelection801B7688(selection)->primaryCount;
    unsigned int requiredSlots = ~(0xFFFFFFFFu << primaryCount);
    return ((requiredSlots & coveredSlots) - requiredSlots) == 0;
}

#pragma push
#pragma use_lmw_stmw on
// Apply IFF model-hide masks to the other loaded avatar parts.
void AvatarModel_ApplyPartHideMasks(void* avatar, int visibilityMode) {
    int coveringSlot;
    int one;
    char* coveringPartCursor;
    char* partCursor;
    char* puppetCursor;
    void* coveringPart;
    int coveredSlot;
    coveringSlot = 0;
    one = 1;
    coveringPartCursor = (char*)avatar;
    coveringPart = 0;
    coveredSlot = 0;
    for (; coveringSlot < ((AvatarPartCountFields*)avatar)->partCount; coveringSlot++, coveringPartCursor += 8) {
        if (!((AvatarPartBindingFields*)coveringPartCursor)->resource)
            continue;
        coveringPart = ((AvatarPartBindingFields*)coveringPartCursor)->part;
        if (!coveringPart || !((IFF_STRUCT::sPart*)coveringPart)->modelHideMask)
            continue;
        puppetCursor = (char*)avatar;
        partCursor = (char*)avatar;
        for (coveredSlot = 0; coveredSlot < ((AvatarPartCountFields*)avatar)->partCount;
             coveredSlot++, puppetCursor += 4, partCursor += 8) {
            unsigned int mask = (unsigned int)one << coveredSlot;
            if (!(((IFF_STRUCT::sPart*)coveringPart)->modelHideMask & mask))
                continue;
            if (coveringSlot == coveredSlot)
                continue;
            if (!((AvatarPartBindingFields*)partCursor)->resource)
                continue;
            if (!((AvatarPartBindingFields*)partCursor)->part)
                continue;
            void* item = ((AvatarPartPuppetFields*)puppetCursor)->puppet;
            if (item)
                WPuppet_SetBoneVisibilityRecursive(item, 1, visibilityMode, 0);
        }
    }
}
#pragma use_lmw_stmw reset
#pragma pop

extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*);
// ORIGINAL: the Wii passes a short deleting flag n = -1 in r4 (`li r4,-1; extsh r4,r4`: free the array, not the guard).
extern void __dt__Q33std6detail16char_array_guardFv(void*, short);
}

// ---- Inlined library code: the std::length_error ("vector length error") throw used by vector growth ----
// Not game logic. BufferGuard_/PtrBuf_801B7028 keep their address-suffixed names because
// their destructors' linker symbols embed the struct name (LogicErrorView = logic_error base, LengthErrorBaseView = message-copying base,
// BufferGuard = temporary message-array guard, PtrBuf = vector split buffer). These structs rebuild the Metrowerks std::length_error object (copied message in a shared
// control block, logic_error vtable then length_error vtable) so the compiler emits the same throw sequence.
// The std::length_error defined below exists only for that purpose in this unit. To read the game code,
// treat ThrowVectorLengthError()/THROW_VECTOR_LENGTH_ERROR as `throw std::length_error("vector length error")`.
struct ExcAllocToken {
    char value;
    char pad[3];
    // Initialize the allocator byte passed to exception-string construction.
    ExcAllocToken() : value(0) {}
};
struct BufferGuard_801B7028 {
    char* data;
    void* allocator;
    // Temporarily own the message allocation until a control block takes it.
    BufferGuard_801B7028(char* p, void* a) : data(p), allocator(a) {}
    // Transfer the guarded array without freeing it.
    char* release() {
        char* p = data;
        data = 0;
        return p;
    }
    // Free a message array left owned on an exceptional exit.
    ~BufferGuard_801B7028() {
        if (data)
            __dla__FPv(data);
    }
};
struct SharedMessageBlock {
    void* table;
    int a, b;
    char* data;
    // Start the shared message control block with both reference counts at one.
    SharedMessageBlock(char* p) : a(1), b(1) {
        table = (void*)s_sharedCharDeleterVtable;
        data = p;
    }
};
struct LogicErrorView {
    void* table;
    // Install the logic-error vtable for the exception base.
    LogicErrorView() {
        table = (void*)__vt__Q23std11logic_error;
    }
    // Run the standard exception-base destructor.
    ~LogicErrorView() {
        __dt__Q23std9exceptionFv(this);
    }
};
struct LengthErrorBaseView : LogicErrorView {
    char* data;
    SharedMessageBlock* owner;
    // Deep-copy diagnostic text under an array guard, then share its ownership.
    LengthErrorBaseView(const char* text) {
        ExcAllocToken allocator;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufferGuard_801B7028 guard(p, &allocator);
            owner = new SharedMessageBlock(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
namespace std {
struct length_error : LengthErrorBaseView {
    // Finish the owned diagnostic with the length-error vtable.
    length_error(const char* p) : LengthErrorBaseView(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Invoke the ABI cleanup for the exception's owned character array.
    ~length_error() {
        __dt__Q33std6detail16char_array_guardFv(this, -1);
    }
    DECOMP_CT_MEMBER(length_error)
};
}

// Throw the supplied length_error using the Metrowerks exception ABI.
inline void ThrowLengthError(const std::length_error& e) {
    __throw(petModelResourceStrings + kPetTextLengthErrorType, (void*)&e, __dt__Q23std12length_errorFv);
}
// Construct and throw the resource-vector length_error with its original storage lifetime.
inline void ThrowVectorLengthError() {
    char* what = petModelResourceStrings + kPetTextVectorLengthError;
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(what);
        exception_runtime::__throw(exception_runtime::petModelResourceStrings + kPetTextLengthErrorType, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
// Keep this expansion local: moving its throw into a helper changes CodeWarrior scheduling.
#define THROW_VECTOR_LENGTH_ERROR()                                                                                 \
    do {                                                                                                            \
        char* what = petModelResourceStrings + kPetTextVectorLengthError;                                           \
        do {                                                                                                        \
            union {                                                                                                 \
                double alignment;                                                                                   \
                unsigned char bytes[sizeof(std::length_error)];                                                     \
            } storage;                                                                                              \
            std::length_error* error = (std::length_error*)storage.bytes;                                           \
            error->__ct(what);                                                                                      \
            exception_runtime::__throw(exception_runtime::petModelResourceStrings + kPetTextLengthErrorType, error, \
                                       (void*)exception_runtime::__dt__Q23std12length_errorFv);                     \
        } while (0);                                                                                                \
    } while (0)

// Pointer vector {begin, size, capacity} embedded in the puppet load request (animation resource handles).
struct PtrVecView {
    void** begin;
    unsigned int size;
    unsigned int cap;
};
// Temporary replacement array used while the vector grows (std::vector's split-buffer helper).
struct PtrBuf_801B7028 {
    void** begin;
    unsigned int size;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int lead;
    // Guard a temporary pointer-vector allocation during growth.
    PtrBuf_801B7028(unsigned int* c) : begin(0), size(0), cap(0), capRef(c), lead(0) {}
    // Free backing storage only; transferred resource handles are not released.
    ~PtrBuf_801B7028() {
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};

extern "C++" {
// Return the larger growth amount by reference without copying it.
inline const unsigned int& vec_max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
}
// Test pointer-slot capacity against the used element count.
inline bool vec_has_room(unsigned int cap, unsigned int size) {
    return size < cap;
}

#pragma push
#pragma use_lmw_stmw on

#pragma inline_depth(16)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Choose vector reserve capacity using the original three-fifths/half/saturating growth policy.
// Scratch references retain caller lifetimes; no allocation or resource ownership changes here.
static inline void ChooseAnimationVectorCapacity(unsigned int& newCapacity, unsigned int& missingSlots,
                                                 int& growthThreeFifths, unsigned int& growthHalf) {
    if (newCapacity < kSmallAnimationVectorLimit) {
        growthThreeFifths = (newCapacity + 1) * 3 / 5;
        newCapacity += vec_max(growthThreeFifths, missingSlots);
    } else if (newCapacity < kMediumAnimationVectorLimit) {
        growthHalf = (1 + newCapacity) / 2;
        newCapacity += vec_max(growthHalf, missingSlots);
    } else {
        newCapacity = kMaxAnimationHandles;
    }
}

extern "C" {
// Finish a pending puppet load, append its resource, and rebuild bounds for the animation list.
// Return 0 only while the manager still reports a pending read; return 1 for
// an inactive request or after finalizing, even if the completed load is null.
// animationResources owns manager handles; the temporary vector owns only
// pointer-array storage. loadedResource aliases the last appended handle.
int PuppetResourceList_PollLoadAndUpdateBounds(void* object) {
    // Phase 1: wait for the manager, then take its completed handle.
    unsigned int* requestWords = (unsigned int*)object;
    if (*(int*)requestWords != kPuppetLoadPending)
        return 1;
    if (!ResourceManager_PollResourceRead(g_resourceManager))
        return 0;
    void* resource = ResourceManager_PollPuppetResourceLoad(g_resourceManager);
    // Phase 2: append the completed handle; the split buffer owns allocation storage only.
    if (resource) {
        PtrVecView* animations = (PtrVecView*)&((PuppetLoadFields*)requestWords)->animationResources;
        unsigned int oldCount = *(volatile unsigned int*)&animations->size;

        if (vec_has_room((animations->cap), oldCount)) {
            unsigned int newCount = (1 + oldCount);
            animations->size = newCount;
            animations->begin[newCount - 1] = resource;
        } else {
            int growthThreeFifths;
            int replacementCapacity;
            unsigned int replacementCount;
            if (kMaxAnimationHandles - animations->cap < 1)
                ThrowVectorLengthError();
            PtrBuf_801B7028 replacement(&animations->cap);
            unsigned int missingSlots = (1 + animations->size) - animations->cap;
            unsigned int newCapacity = animations->cap;
            if (missingSlots > kMaxAnimationHandles - newCapacity) {
                THROW_VECTOR_LENGTH_ERROR();
            }

            // Growth policy (inlined vector reserve): capacity below kSmallAnimationVectorLimit grows by
            // 3/5 of (cap+1), below kMediumAnimationVectorLimit by half, otherwise jumps to the maximum;
            // each growth step is at least the number of missing slots (`missingSlots`).
            unsigned int growthHalf;
            ChooseAnimationVectorCapacity(newCapacity, missingSlots, growthThreeFifths, growthHalf);
            if (newCapacity > kMaxAnimationHandles)
                THROW_VECTOR_LENGTH_ERROR();
            // Reserve, place the new tail, then copy existing handles in order.
            replacement.begin = (void**)appMallocDummy__FUl(newCapacity * 4);
            replacement.lead = animations->size;
            (replacement.begin + replacement.lead)[replacement.size++] = resource;
            int copiedCount;
            void** oldHandles = animations->begin;
            unsigned int copiedBytes;

            replacement.cap = newCapacity;
            copiedCount = (int)((unsigned int)(oldHandles + animations->size) - (int)oldHandles) / 4;
            replacement.lead -= copiedCount;
            copiedBytes = (4 * copiedCount);
            memcpy(replacement.begin + replacement.lead, oldHandles, copiedBytes);
            memset(oldHandles, 0, copiedBytes);
            replacement.size += copiedCount;
            void** newHandles;

            replacementCount = replacement.size;
            unsigned int oldCapacity;
            oldCapacity = animations->cap;
            newHandles = replacement.begin;
            replacementCapacity = replacement.cap;
            animations->size = 0;

            // Commit the new allocation; replacement now frees only the old pointer array.
            replacement.begin = animations->begin;
            animations->cap = replacementCapacity;
            replacement.cap = oldCapacity;
            animations->size = replacementCount;
            animations->begin = newHandles;
            replacement.size = 0;
        }
        requestWords[kLoadCompletedResourceWord] = (unsigned int)resource;
    }
    // Phase 3: no animations -> puppet bounds; otherwise reset bounds from
    // the first animation and union subsequent animation bounds into them.
    unsigned int* begin = (unsigned int*)requestWords[kLoadAnimationsWord];
    if (begin == (unsigned int*)requestWords[kLoadAnimationsWord] + requestWords[kLoadAnimationCountWord])
        WPuppet_UpdateBoundsFromResourceBox((void*)requestWords[kLoadPuppetWord], 0, 1);
    else {
        WPuppet_AccumulateAnimationBounds((void*)requestWords[kLoadPuppetWord], 1, *begin, 1, 0, 0);
        begin = begin + 1;
        while (begin != (unsigned int*)requestWords[kLoadAnimationsWord] + requestWords[kLoadAnimationCountWord]) {
            WPuppet_AccumulateAnimationBounds((void*)requestWords[kLoadPuppetWord], 0, *begin, 1, 0, 0);
            ++begin;
        }
    }
    // Phase 4: publish completion only after all bounds have been rebuilt.
    OSReport(petModelResourceStrings + kPetTextReplaceComplete);
    requestWords[kLoadStateWord] = kPuppetLoadIdle;
    return 1;
}
#pragma pop

// Compare character, variants, counts, and all 24 outfit part IDs.
int ItemSelection_Equals(unsigned int* left, unsigned int* right) {
    if (left[0] != right[0] || OutfitHairVariant(left) != OutfitHairVariant(right) ||
        OutfitBaseVariant(left) != OutfitBaseVariant(right) ||
        AsItemSelection801B7688(left)->primaryCount != AsItemSelection801B7688(right)->primaryCount ||
        AsItemSelection801B7688(left)->totalCount != AsItemSelection801B7688(right)->totalCount)
        return 0; // original +6, +7, +4, +5
    unsigned int* leftCursor = left;
    unsigned int* rightCursor = right;
    int unusedLoopAccumulator = 0; // Each group below compares eight part IDs.
    for (int group = 0; group < 3; group++) {
        if (leftCursor[2] != rightCursor[2])
            return 0;
        if (leftCursor[3] != rightCursor[3])
            return 0;
        if (leftCursor[4] != rightCursor[4])
            return 0;
        if (leftCursor[5] != rightCursor[5])
            return 0;
        if (leftCursor[6] != rightCursor[6])
            return 0;
        if (leftCursor[7] != rightCursor[7])
            return 0;
        if (leftCursor[8] != rightCursor[8])
            return 0;
        if (leftCursor[9] != rightCursor[9])
            return 0;
        rightCursor += 8;
        leftCursor += 8;
        unusedLoopAccumulator += 7;
    }
    (void)unusedLoopAccumulator;
    return 1;
}

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
enum {
    kPetalCreateEntriesSlot = 6
};
// Initialize petal wrapping bounds and allocate entries with display scale one.
void* ntPetal_Ctor(void* emitterStorage, void* particleCountBits) {
    // Shared consumers establish gravity, coordinate-wrap bounds, and owned particle storage.
    PetalSilicaEmitterView* emitter = (PetalSilicaEmitterView*)emitterStorage;
    float lowerBound[3];
    float lowerBoundDirection[3];
    float upperBound[3];
    float upperBoundDirection[3];
    emitter->gravityScale = kFloat_1_PetEffects;
    emitter->vtbl = (void*)__vt__7ntPetal;
    Vec3_Zero(emitter->boundsMin);
    Vec3_Zero(emitter->boundsMax);
    emitter->particles = 0;
    lowerBoundDirection[0] = kFloat_1_PetEffects;
    lowerBoundDirection[1] = kFloat_0_7_PetEffects;
    lowerBoundDirection[2] = kFloat_1_PetEffects;
    // Lower camera-relative world-space wrap bound = -112 * (1, 0.7, 1); shared constant also serves silica particles.
    __ml__FfRC7WVector(lowerBound, lowerBoundDirection, kFloat_Neg112_PetParticleExtent);
    __as__7WVectorFRC7WVector(emitter->boundsMin, lowerBound);
    upperBoundDirection[0] = kFloat_1_PetEffects;
    upperBoundDirection[1] = kFloat_0_3_PetEffects;
    upperBoundDirection[2] = kFloat_1_PetEffects;
    // Upper camera-relative world-space wrap bound = 112 * (1, 0.3, 1); this is an emitter extent, not spin display.
    __ml__FfRC7WVector(upperBound, upperBoundDirection, kFloat_112_PetParticleExtent);
    __as__7WVectorFRC7WVector(emitter->boundsMax, upperBound);
    void** petalVtable = *(void***)emitterStorage;
    // ntPetal table word 6 = ntPetal_RecreateParticleEntries: recreate entries using count particleCountBits and display scale 1.
    // particleCountBits carries the count bits through this existing pointer-typed C ABI.
    ((void (*)(void*, void*, float))petalVtable[kPetalCreateEntriesSlot])(emitterStorage, particleCountBits,
                                                                          kFloat_1_PetEffects);
    return emitterStorage;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Build an outfit from explicit part IDs plus uncovered default slots.
// requestedPartIds must cover defaultPartCount+optionalPartCount entries;
// it is borrowed and must not alias object, whose old part array is cleared.
// Return 0 for missing character or invalid explicit part, 1 after publishing
// character/variant/count metadata. This is NOT transactional: after character
// lookup succeeds it clears old parts and writes validated slots as it goes;
// a later failure leaves those changes with the OLD metadata still present.
int ItemSelection_BuildCharacterOutfit(void* object, unsigned int characterId, unsigned int hairVariant,
                                       unsigned int baseVariant, void* requestedPartIds, int unused) {
    void* characterRecord;
    unsigned int* requestedParts;
    unsigned int* outfitWords;
    unsigned int characterBits;
    unsigned int coveredSlots;
    int slot;
    unsigned int slotBits;
    int firstSlotBit;
    characterRecord = ItemInventory_FindCharacterData(g_itemSelectionInventory, characterId);
    if (!characterRecord)
        return 0;
    // Phase 1: clear previous part slots, validate explicit parts, collect hides.
    // Payload view: outfitWords[0] = character id, byte [5] = totalCount (part slots to clear), and part-ID
    // slot k lives at outfitWords[2 + k] (slot index = position in the character's default+optional parts).
    // An entry's hideMask marks other slots as covered so the default fill below skips them.
    requestedParts = (unsigned int*)requestedPartIds;
    outfitWords = (unsigned int*)object;
    if (((unsigned char*)outfitWords)[5])
        memset(outfitWords + 2, 0, ((unsigned char*)outfitWords)[5] * 4);
    characterBits = characterId & kItemKindPayloadMask;
    coveredSlots = 0;
    slot = 0;
    for (; slot < (int)(((IFF_STRUCT::sChar*)characterRecord)->defaultPartCount +
                        ((IFF_STRUCT::sChar*)characterRecord)->optionalPartCount);
         ++slot) {
        if (!requestedParts[slot])
            continue;
        if (((requestedParts[slot] >> kPartCharacterShift) & kPartCharacterMask) != characterBits)
            return 0;
        IFF_STRUCT::sPart* entry =
            (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_itemSelectionInventory, requestedParts[slot]);
        if (!entry)
            return 0;
        outfitWords[2 + slot] = requestedParts[slot];
        coveredSlots |= entry->hideMask;
    }
    outfitWords = (unsigned int*)object;
    // Phase 2: fill default slots not covered by the explicit parts' hide masks.
    // The default lookup result is ignored; it does not gate publication.
    characterBits = (characterBits << kPartCharacterShift) | kPartKind;
    slotBits = 0;
    firstSlotBit = 1;
    for (slot = 0; slot < ((IFF_STRUCT::sChar*)characterRecord)->defaultPartCount; ++slot) {
        if (!((firstSlotBit << slot) & coveredSlots)) {
            unsigned int defaultPartId = characterBits | slotBits | (kDefaultPart | kDefaultSubPart);
            if (!slot)
                defaultPartId |= hairVariant;
            ItemInventory_FindPartData(g_itemSelectionInventory, defaultPartId);
            outfitWords[2 + slot] = defaultPartId;
        }
        slotBits += kPartSlotStride;
    }
    // Phase 3: publish the new metadata, then perform the final part lookups.
    ((unsigned int*)object)[0] = characterId;
    OutfitHairVariant(object) = hairVariant;
    OutfitBaseVariant(object) = baseVariant;
    AsItemSelection801B7688(object)->primaryCount = ((IFF_STRUCT::sChar*)characterRecord)->defaultPartCount;
    AsItemSelection801B7688(object)->totalCount = ((IFF_STRUCT::sChar*)characterRecord)->defaultPartCount +
                                                  ((IFF_STRUCT::sChar*)characterRecord)->optionalPartCount;
    requestedParts = (unsigned int*)requestedPartIds;
    slot = 0;
    for (; slot < (int)(((IFF_STRUCT::sChar*)characterRecord)->defaultPartCount +
                        ((IFF_STRUCT::sChar*)characterRecord)->optionalPartCount);
         ++slot)
        if (requestedParts[slot])
            ItemInventory_FindPartData(g_itemSelectionInventory, requestedParts[slot]);
    return 1;
}
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma optimization_level 3
// Validate and swap the character base-bitmap variant while releasing prior bitmap handles.
// Return 1 if parts are not ready (no work), or after installing a replacement;
// return 0 on validation/load failure. With selection, its byte 7 and avatar
// variant are changed BEFORE I/O and are not rolled back on failure.
// The character record's shared filename is edited in place: last pre-dot
// byte becomes baseVariant+'1', restored to literal '1' ONLY after success.
// Failures after loading originalBitmap do not release it here; a failed
// replacement also leaves the edited filename. These are existing behaviors.
int CharacterModel_ApplyBitmapVariant(void* avatar, void* selection, unsigned char baseVariant) {
    if (!((AvatarAppearanceFields*)avatar)->partsReady)
        return 1;
    if (selection) {
        unsigned int characterId = *(unsigned int*)selection;
        int ok;
        if (((PuppetLoadFields*)avatar)->characterId != characterId)
            return 0;
        if (((AvatarAppearanceFields*)avatar)->characterId != characterId) {
            ok = 0;
        } else {
            unsigned char* selectionBytes = (unsigned char*)selection;
            unsigned int savedVariant = selectionBytes[7];
            if (((AvatarAppearanceFields*)avatar)->baseVariant != savedVariant) {
                ok = 0;
            } else if (savedVariant == baseVariant) {
                ok = 0;
            } else {
                selectionBytes[7] = baseVariant;
                ok = 1;
                ((AvatarAppearanceFields*)avatar)->baseVariant = baseVariant;
            }
        }
        if (!ok)
            return 0;
    }
    struct Res {
        char pad[0xAC];
        signed char name[1];
    };
    void* recordOrBitmap = ((void* (*)(void*, void*))ItemInventory_FindCharacterData)(
        ((CSharedDoc*)g_sharedDoc)->itemInventory,
        (void*)((PuppetLoadFields*)avatar)->characterId); // original inventory +0xA04
    if (!recordOrBitmap)
        return 0;
    // Load the original name for pair substitution, then mutate the shared
    // name for the replacement. The avatar retains only the replacement.
    Res* baseTextureRecord = (Res*)recordOrBitmap;
    if (!baseTextureRecord->name[0])
        return 0;
    void* originalBitmap = ResourceManager_LoadBitmap(g_resourceManager, (char*)baseTextureRecord->name, 0, 0, 0);
    if (!originalBitmap)
        return 0;
    char* dot = strrchr((char*)baseTextureRecord->name, '.');
    if (!dot || dot == (char*)baseTextureRecord->name)
        return 0;
    *(dot - 1) = baseVariant + '1';
    recordOrBitmap = ResourceManager_LoadBitmap(g_resourceManager, (char*)baseTextureRecord->name, 0, 0, 0);
    if (!recordOrBitmap)
        return 0;
    WPuppet_ApplyNamedResourcePairRecursive(((PuppetLoadFields*)avatar)->puppet, recordOrBitmap, originalBitmap, 0);
    *(dot - 1) = '1';
    if (g_resourceManager && originalBitmap)
        ResourceManager_ReleaseId(g_resourceManager, originalBitmap);
    void* previousBitmap = ((PuppetLoadFields*)avatar)->baseBitmap;
    if (previousBitmap)
        ResourceManager_ReleaseId(g_resourceManager, previousBitmap);
    ((PuppetLoadFields*)avatar)->baseBitmap = recordOrBitmap;
    return 1;
}
#pragma optimization_level reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Change an ASCII decimal digit only if the resource name contains a digit at that position.
static inline void SetHairVariantDigit(char* digit, int replacement) {
    if (*digit >= '0' && *digit <= '9')
        *digit = replacement;
}
// Preserve the signed-char test used for the tens digit in shared IFF storage.
static inline void SetHairVariantTensDigit(signed char* digit, int replacement) {
    if (*digit >= '0' && *digit <= '9')
        *digit = replacement;
}

// Apply the matching part and character hair texture variants, retaining the resource-load order.
// Part replacement runs BEFORE character/selection validation and can persist
// even if this later returns 0. Return value reports hair replacement only.
// Part name edits use local path[40]; hair name edits mutate the shared IFF
// record, restoring numeric suffix to 01 only when a replacement loads.
// Failed hair load can therefore leave a changed filename and variant bytes.
// Successful replacements release the old avatar handle, store the new one,
// and release the original pair-matching bitmap; no ownership rollback occurs.
int AvatarModel_ApplyTextureVariant(void* avatar, void* selection, unsigned char hairVariant) {
    unsigned int partId;
    unsigned char partCount;
    unsigned int* partCursor;
    int partIndex;
    unsigned int characterId;
    unsigned int characterIndex;
    unsigned int partSlot;
    void* originalPartBitmap;
    void* partRecord;
    char partVariantPath[40];
    char* partExtension;
    int textureNumber;
    void* replacementBitmap;
    void* characterRecord;
    char* hairExtension;
    char* biasedHairNameBase;
    void* originalHairBitmap;
    int result;

    // Phase 1: replace the first part in the character-specific variant slot.
    if (selection) {
        partCount = AsItemSelection801B7688(selection)->totalCount; // original +5
        partCursor = (unsigned int*)selection;
        for (partIndex = 0; partIndex < partCount; partCursor++, partIndex++) {
            characterId = *(unsigned int*)selection;
            partId = partCursor[2];
            characterIndex = characterId & kItemKindPayloadMask;
            partSlot = (partId >> kPartSlotShift) & kPartSlotMask;
            if (partSlot == s_characterVariantPartSlots[characterIndex]) {
                partRecord = ItemInventory_FindPartData(
                    ((CSharedDoc*)g_sharedDoc)->itemInventory,
                    (&AsItemSelection801B7688(selection)->firstPartId)[partIndex]); // original inventory +0xA04
                if (!partRecord)
                    break;
                originalPartBitmap = ResourceManager_LoadBitmap(
                    g_resourceManager, ((IFF_STRUCT::sPart*)partRecord)->secondTextures[0], 0, 0, 0);
                strcpy(partVariantPath, ((IFF_STRUCT::sPart*)partRecord)->secondTextures[0]);
                partExtension = strrchr(partVariantPath, '.');
                if (!partExtension)
                    return 0;
                textureNumber = String_ParseDecimalInt(partExtension - 2);
                sprintf(partExtension - 2, petModelResourceStrings + kPetTextBitmapVariantFormat,
                        textureNumber + hairVariant);
                replacementBitmap = ResourceManager_LoadBitmap(g_resourceManager, partVariantPath, 0, 0, 0);
                WPuppet_ApplyNamedResourcePairRecursive(((PuppetLoadFields*)avatar)->puppet, replacementBitmap,
                                                        originalPartBitmap, 0);
                if (((PuppetLoadFields*)avatar)->partBitmap)
                    ResourceManager_ReleaseId(g_resourceManager, ((PuppetLoadFields*)avatar)->partBitmap);
                ((PuppetLoadFields*)avatar)->partBitmap = replacementBitmap;
                if (g_resourceManager && originalPartBitmap)
                    ResourceManager_ReleaseId(g_resourceManager, originalPartBitmap);
                break;
            }
        }
    }
    // Phase 2: require both character IDs before publishing variant metadata.
    // Part replacement above may already have changed the avatar when either check fails.
    if (selection) {
        unsigned int selectedCharacterId = *(unsigned int*)selection;
        int characterMatches;
        if (((PuppetLoadFields*)avatar)->characterId != selectedCharacterId)
            return 0;
        if (((AvatarAppearanceFields*)avatar)->characterId != selectedCharacterId) {
            characterMatches = 0;
        } else {
            OutfitHairVariant(selection) = hairVariant;
            characterMatches = 1;
            ((AvatarAppearanceFields*)avatar)->hairVariant = hairVariant;
        }
        if (!characterMatches)
            return 0;
    }
    // Phase 3: after publishing variant metadata, load the character hair pair.
    // The tens-digit address is characterRecord + (hairExtension - hairTexture - 2)
    // + 0x84. Keep this byte expression for the original layout-dependent arithmetic.
    characterRecord = ((void* (*)(void*, void*))ItemInventory_FindCharacterData)(
        ((CSharedDoc*)g_sharedDoc)->itemInventory,
        (void*)((PuppetLoadFields*)avatar)->characterId); // original inventory +0xA04
    if (!characterRecord)
        return 0;
    result = 0;
    originalHairBitmap =
        ResourceManager_LoadBitmap(g_resourceManager, ((IFF_STRUCT::sChar*)characterRecord)->hairTexture, 0, 0, 0);
    if (originalHairBitmap) {
        hairExtension = strrchr(((IFF_STRUCT::sChar*)characterRecord)->hairTexture, '.');
        if (hairExtension && hairExtension != ((IFF_STRUCT::sChar*)characterRecord)->hairTexture) {
            SetHairVariantDigit(hairExtension - 1, (hairVariant + 1) % 10 + '0');
            biasedHairNameBase = (char*)characterRecord +
                                 ((hairExtension - 1) - (((IFF_STRUCT::sChar*)characterRecord)->hairTexture + 1));
            SetHairVariantTensDigit((signed char*)biasedHairNameBase + 0x84, (hairVariant + 1) / 10 + '0');
            replacementBitmap = ResourceManager_LoadBitmap(g_resourceManager,
                                                           ((IFF_STRUCT::sChar*)characterRecord)->hairTexture, 0, 0, 0);
            if (replacementBitmap) {
                WPuppet_ApplyNamedResourcePairRecursive(((PuppetLoadFields*)avatar)->puppet, replacementBitmap,
                                                        originalHairBitmap, 0);
                SetHairVariantDigit(hairExtension - 1, '1');
                SetHairVariantTensDigit((signed char*)biasedHairNameBase + 0x84, '0');
                if (((PuppetLoadFields*)avatar)->hairBitmap)
                    ResourceManager_ReleaseId(g_resourceManager, ((PuppetLoadFields*)avatar)->hairBitmap);
                ((PuppetLoadFields*)avatar)->hairBitmap = replacementBitmap;
                result = 1;
            }
        }
    }
    if (g_resourceManager && originalHairBitmap)
        ResourceManager_ReleaseId(g_resourceManager, originalHairBitmap);
    return result;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
#pragma pop
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046217C", _ctors_u801B6670_0, AvatarModel_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046217C", _ctors_u801B6670_1, PetalEffect_InitAngleConstant);
#pragma pop

// Verify the partial views against the offsets used by this unit.
typedef char
    PuppetLoadFields_animationResources_offset[((int)&((PuppetLoadFields*)0)->animationResources == 0x1c) ? 1 : -1];
typedef char PuppetLoadFields_loadedResource_offset[((int)&((PuppetLoadFields*)0)->loadedResource == 0x28) ? 1 : -1];
typedef char
    AvatarAppearanceFields_partsReady_offset[((int)&((AvatarAppearanceFields*)0)->partsReady == 0x1b8) ? 1 : -1];
typedef char
    AvatarAppearanceFields_hairVariant_offset[((int)&((AvatarAppearanceFields*)0)->hairVariant == 0x1c2) ? 1 : -1];
typedef char
    AvatarAppearanceFields_baseVariant_offset[((int)&((AvatarAppearanceFields*)0)->baseVariant == 0x1c3) ? 1 : -1];
typedef char
    AvatarAttachmentFields_attachments_offset[((int)&((AvatarAttachmentFields*)0)->attachments == 0x284) ? 1 : -1];
typedef char AvatarPartBindingFields_resource_offset[((int)&((AvatarPartBindingFields*)0)->resource == 0x1c4) ? 1 : -1];
typedef char AvatarPartBindingFields_part_offset[((int)&((AvatarPartBindingFields*)0)->part == 0x1c8) ? 1 : -1];

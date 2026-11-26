// Character and caddie preview operations for the shops: opening the shared
// preview resources, building the avatar or pet preview, and translating the
// saved outfit and the selected shop item into equipment changes.
//
// The shared base operation (resource handle, position table, exhibition and
// save record) and its plain C entry points are defined here too.
#include <decomp/static_init.h>
#include <decomp/local_class.h>
typedef unsigned long new_size_t;
#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CSharedDoc.h"
#include "classes/CExhibition.h"
#include "classes/CModelOperationCaddie.h"
#include "classes/CModelOperation.h"
#include "classes/CModelOperationChara.h"
#include "classes/CExhibition_model_pos.h"
#include "classes/CAsyncExhibition.h"
#include "classes/CAsyncExhibitionAvatar.h"
#include "classes/CAsyncExhibitionPet.h"
#include <decomp/cw_bitfields.h>
#include "classes/CSharedFileHandle.h"

enum PreviewItemKind {
    kPartItem = 2,
    kCaddieItem = 7,
    kCaddieEquipmentItem = 8,
    kHairStyleItem = 15,
    kAuxPartItem = 28,
    kItemKindShift = 26
};
enum PreviewItemIdBits {
    kCaddieIndexShift = 21,
    kCaddieIndexMask = 0x1F,
    kCaddieItemSerialMask = 0x1FFF,
    kCaddieCount = 8,
    kOutfitPartCount = 24,
    kNoInventoryIndex = -1
};
// Views of the shop save data this file reads and writes: the outfit rows of
// the player save and the equipment record handed to the avatar preview.
struct SavedPreviewOutfit {
    unsigned char flags, alignmentPadding;
    short partIndices[kOutfitPartCount];
    signed char hairStyleIndex, auxPartIndex;
};
struct PreviewSaveRecord {
    char prefix[0x2C];
    signed char characterIndex;
    char selectedClubBallCaddie[3];
    signed char caddieItemIndex;
    char inventoryAndCourseRecords[0x29B];
    SavedPreviewOutfit outfits[8]; // 0x2CC
};
typedef char SavedPreviewOutfit_size[(sizeof(SavedPreviewOutfit) == 0x34) ? 1 : -1];
typedef char PreviewSaveRecord_outfits[((int)&((PreviewSaveRecord*)0)->outfits == 0x2CC) ? 1 : -1];
struct ResolvedPreviewEquipment {
    unsigned int characterId;
    int characterIndex;
    unsigned int partIds[kOutfitPartCount];
    unsigned char appearance[0x2C]; // starts at +0x68; hair colour occupies the upper five bits
};

extern "C" {
void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
}
inline void* operator new(new_size_t size) {
    return __nw__FUl(size);
}
inline void* operator new(new_size_t, void* p) throw() {
    return p;
}
void operator delete(void*) throw();

extern "C" {

void* memcpy(void*, const void*, unsigned long);
void* memset(void*, int, unsigned long);

extern const float kFloat_6_2831855;
extern float s_modelOperationFirstAnglePeriod;
extern float s_modelOperationSecondAnglePeriod;
extern char caddiePreviewResourcePath[];
extern char modelOperationBaseVtable[];
extern char __vt__20CModelOperationChara[];
extern char characterPreviewResourcePath[];

void* __dt__18ModelOperationBaseFv(void*, int);
void __dl__FPv(void*);
int ItemInventory_FindAuxPartIndex(void*, unsigned int);
unsigned char* ItemInventory_FindHairStyleData(void*, int);
int ItemInventory_FindHairStyleIndex(void*, unsigned int);
int ItemInventory_FindPartIndex(void*, int);
int SharedFile_Create(void*, int);
int CSharedFilePtr_GetLoadedBuffer(void*);
void ItemSelection_Init(void*);
extern const float kFloat_12_ModelPreview;
extern const float kFloat_10_ModelPreview;
extern char __vt__21CExhibition_model_pos[];
extern CSharedDoc* g_sharedDoc;
int ModelOperationBase_GetDefaultViewport(void*, void*);
int ModelOperationBase_ReleaseExhibition(void*);
int ItemInventory_FindCaddieIndex(void*, unsigned int);
int Memory_Clear(void*, int);
int ItemInventory_GetCharacterIdAt(void*, int);
int ItemInventory_GetPartIdAt(void*, int);
int ItemInventory_GetHairStyleIdAt(void*, int);
int ModelOperationBase_ApplyCameraPreset(void*, int);
void* ItemInventory_FindCharacterIndex(void*, void*); // (table, key): the key is the caller's own argument
void* ItemInventory_GetAuxPartIdAt(void*, int);
int ItemInventory_GetCaddieItemIdAt(void*, int);
int ItemInventory_GetCaddieIdAt(void*, int);
int ItemInventory_FindCaddieItemIndex(void*, unsigned int);
int ItemSelection_BuildOutfitFromRecord(void*, void*);
int ItemSelection_Validate(void*);
void ItemSelection_RefreshPrimaryItems(void*);

// Open the shared character-preview resource through the base operation.
int CModelOperationChara::OpenResource() {
    CModelOperation::OpenResource(characterPreviewResourcePath);
    return 1;
}

// Reject non-part ids, then apply validated character part changes to the exhibition.
int CModelOperationChara::ApplyPartChanges(unsigned int partTypeId) {
    if ((partTypeId >> kItemKindShift) != kPartItem || partTypeId == 0) {
        return -1;
    }
    return (int)((CAsyncExhibition*)asyncExhibition)->CopyValidatedItemChanges(&selection, partTypeId);
}

// Return whether the model operation's shared-file handle exposes a loaded buffer.
int ModelOperationBase_IsResourceLoaded(void* self) {
    int loadedBuffer = CSharedFilePtr_GetLoadedBuffer((*(void**)&((CModelOperation*)self)->sharedFile));
    switch (((unsigned int)(-loadedBuffer | loadedBuffer)) >> 31) {
    default:
        return 1;
    case 0:
        return 0;
    }
}

// Clear the caddie-equipment selection before applying the requested item.
int CModelOperationCaddie::SetItem(int typeId) {
    inventoryIndex = -1;
    return ToggleItem((unsigned int)typeId);
}

// Initialize the first model-operation angle-period constant to two pi.
void ModelOperation_InitFirstAnglePeriod() {
    s_modelOperationFirstAnglePeriod = kFloat_6_2831855;
}

// Initialize the second model-operation angle-period constant to two pi.
void ModelOperation_InitSecondAnglePeriod() {
    s_modelOperationSecondAnglePeriod = kFloat_6_2831855;
}

// Attach the save-record address used by a model operation.
int ModelOperationBase_SetSaveRecord(void* self, void* saveRecordAddress) {
    ((CModelOperation*)self)->saveData = saveRecordAddress;
    return 1;
}

// Initialize the base operation with no resource, position table, exhibition or save record.
void ModelOperationBase_Ctor(void* self) {
    ((CModelOperation*)self)->sharedFile = 0;
    ((CModelOperation*)self)->vtbl = modelOperationBaseVtable;
    ((CModelOperation*)self)->positionTable = 0;
    ((CModelOperation*)self)->pointerRotationEnabled = 0;
    ((CModelOperation*)self)->asyncExhibition = 0;
    ((CModelOperation*)self)->saveData = 0;
}

// Open the shared caddie-preview resource through the base operation.
int CModelOperationCaddie::OpenResource() {
    ((CModelOperation*)this)->OpenResource(caddiePreviewResourcePath);
    return 1;
}

// Request an avatar model update unless the inventory index is invalid.
int CModelOperationChara::RequestModelUpdate(int modelIndex) {
    if (modelIndex == -1) {
        return 0;
    }
    CAsyncExhibitionAvatar::RequestModelUpdate(asyncAvatar, &selection, modelIndex);
    return 1;
}

// Toggle a ring inventory index and forward the attachment change to the avatar preview.
int CModelOperationChara::ToggleAttachment(int, unsigned int auxPartTypeId) {
    if ((auxPartTypeId >> kItemKindShift) != kAuxPartItem && auxPartTypeId != 0) {
        return 0;
    }
    int* inventory = (int*)((CSharedDoc*)g_sharedDoc)->itemInventory;
    int auxPartIndex = ItemInventory_FindAuxPartIndex(inventory, auxPartTypeId);
    if (auxPartIndex == toggledItemIndex) {
        toggledItemIndex = -1;
    } else {
        toggledItemIndex = auxPartIndex;
    }
    ((CAsyncExhibition*)asyncAvatar)->ToggleAttachment(0, (void*)auxPartTypeId);
    return 1;
}

// Resolve a hair-style id, remember its inventory index and apply its IFF colour index.
int CModelOperationChara::SetHairStyle(unsigned int hairStyleTypeId) {
    if ((hairStyleTypeId >> kItemKindShift) != kHairStyleItem || hairStyleTypeId == 0) {
        return 0;
    }
    IFF_STRUCT::sHairStyle* hairStyleRecord = (IFF_STRUCT::sHairStyle*)ItemInventory_FindHairStyleData(
        ((CSharedDoc*)g_sharedDoc)->itemInventory, hairStyleTypeId);
    unsigned char colourIndex = hairStyleRecord->colourIndex;
    selectedItemIndex = ItemInventory_FindHairStyleIndex(((CSharedDoc*)g_sharedDoc)->itemInventory, hairStyleTypeId);
    struct ExhibitionVtbl {
        virtual void vfunc_0() DECOMP_LOCAL_VIRTUAL;
        virtual void vfunc_1() DECOMP_LOCAL_VIRTUAL;
        virtual void vfunc_2() DECOMP_LOCAL_VIRTUAL;
        virtual int applyColour(void*, unsigned char) DECOMP_LOCAL_VIRTUAL;
    };
    return ((ExhibitionVtbl*)asyncExhibition)->applyColour(&selection, colourIndex);
}

#pragma use_lmw_stmw on
// Store preview part indices, hair and ring selection in the active character outfit.
int CModelOperationChara::WriteEquipmentToSave() {
    CModelOperationChara* partCursor;
    int partByteOffset;
    int characterByteOffset;
    int partSlot;
    partCursor = this;
    partSlot = 0;
    partByteOffset = 0;
    characterByteOffset = ((SavePlayerRecord*)saveData)->characterIndex * 0x34;
    // The cursor advances the object pointer by four bytes per slot, which is
    // how the original reaches the next part id.
    for (; partSlot < kOutfitPartCount; ++partSlot) {
        short partIndex = (short)ItemInventory_FindPartIndex(((CSharedDoc*)g_sharedDoc)->itemInventory,
                                                             partCursor->selection.partIds[0]);
        *(short*)((char*)saveData + characterByteOffset + partByteOffset +
                  IFF_OFFSET(PreviewSaveRecord, outfits[0].partIndices)) = partIndex;
        partCursor = (CModelOperationChara*)((char*)partCursor + 4);
        partByteOffset += 2;
    }
    int hairStyleIndex = selectedItemIndex;
    if (hairStyleIndex != -1)
        *((char*)saveData + characterByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].hairStyleIndex)) =
            (char)hairStyleIndex;
    *((char*)saveData + characterByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].auxPartIndex)) =
        (char)toggledItemIndex;
    return 1;
}
#pragma use_lmw_stmw reset

// Create and remember a shared-file handle for the requested resource path.
int CModelOperation::OpenResource(const char* path) {
    int file = SharedFile_Create((void*)path, 0);
    sharedFile = (void*)file;
    return file != 0;
}

// Select a caddie, apply its camera preset, and clear its saved equipment selection.
int CModelOperationCaddie::SelectCaddie(unsigned int caddieTypeId) {
    if ((caddieTypeId >> kItemKindShift) != kCaddieItem || caddieTypeId == 0)
        return 0;
    void* inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
    int caddieIndex = ItemInventory_FindCaddieIndex(inventory, caddieTypeId);
    if (asyncPet == 0)
        return 0;
    if (ModelOperationBase_ApplyCameraPreset(this, caddieIndex) == 0)
        return 0;
    inventoryIndex = -1;
    ((PreviewSaveRecord*)saveRecord)->caddieItemIndex = -1;
    return ((CAsyncExhibitionPet*)asyncPet)->SetPetSelection((void*)caddieTypeId);
}

// Release the exhibition, shared resource and position table, then optionally free the operation.
void* __dt__18ModelOperationBaseFv(void* self, int deleteStorage) {
    if (self != 0) {
        void* exhibition = (*(void**)&((CModelOperation*)self)->asyncExhibition);
        *(void**)self = modelOperationBaseVtable;
        if (exhibition != 0) {
            if (exhibition != 0) {
                ((void (**)(void*, int)) * (void**)exhibition)[2](exhibition, 1);
            }
            (*(void**)&((CModelOperation*)self)->asyncExhibition) = 0;
        }
        if ((*(void**)&((CModelOperation*)self)->sharedFile) != 0) {
            (int)CSharedFileHandle::Destroy((*(void**)&((CModelOperation*)self)->sharedFile), 1);
            (*(void**)&((CModelOperation*)self)->sharedFile) = 0;
        }
        if ((*(void**)&((CModelOperation*)self)->positionTable) != 0) {
            if ((*(void**)&((CModelOperation*)self)->positionTable) != 0) {
                ((void (**)(void*, int))(*(void**)((char*)(*(void**)&((CModelOperation*)self)->positionTable) + 4)))[2](
                    (*(void**)&((CModelOperation*)self)->positionTable), 1);
            }
            (*(void**)&((CModelOperation*)self)->positionTable) = 0;
        }
        if (deleteStorage > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma use_lmw_stmw on
// Resolve saved part indices and hair colour into the character-preview equipment record.
int CModelOperationChara::BuildEquipmentFromSave(void* output, int characterIndexOrPartSlot) {
    IFF_STRUCT::sHairStyle hairStyle;
    int partByteOffset;
    void* partOutputCursor;
    int characterByteOffset;
    void* inventory;
    CSharedDoc* sharedDoc;
    int typeId;
    Memory_Clear(output, 0x94);
    sharedDoc = (CSharedDoc*)g_sharedDoc;
    inventory = sharedDoc->itemInventory;
    if (inventory == 0)
        return 0;
    typeId = ItemInventory_GetCharacterIdAt(inventory, characterIndexOrPartSlot);
    ((ResolvedPreviewEquipment*)output)->characterIndex = characterIndexOrPartSlot;
    ((ResolvedPreviewEquipment*)output)->characterId = typeId;
    characterByteOffset = characterIndexOrPartSlot * 0x34;
    partOutputCursor = output;
    characterIndexOrPartSlot = 0;
    partByteOffset = 0;
    for (; characterIndexOrPartSlot < kOutfitPartCount; ++characterIndexOrPartSlot) {
        short partIndex = *(short*)((char*)saveData + characterByteOffset + partByteOffset +
                                    IFF_OFFSET(PreviewSaveRecord, outfits[0].partIndices));
        ((ResolvedPreviewEquipment*)partOutputCursor)->partIds[0] = ItemInventory_GetPartIdAt(inventory, partIndex);
        partByteOffset += 2;
        partOutputCursor = (char*)partOutputCursor + 4;
    }
    typeId = ItemInventory_GetHairStyleIdAt(inventory,
                                            (signed char)*((char*)saveData + characterByteOffset +
                                                           IFF_OFFSET(PreviewSaveRecord, outfits[0].hairStyleIndex)));
    if (typeId == 0)
        return 0;
    // The original copies the whole style record before reading its colour byte.
    unsigned int* src = (unsigned int*)ItemInventory_FindHairStyleData(inventory, typeId) - 1;
    unsigned int* dst = (unsigned int*)&hairStyle - 1;
    int count = 0x0C;
    do {
        unsigned int a = src[1];
        unsigned int b = src[2];
        src += 2;
        dst[1] = a;
        dst[2] = b;
        dst += 2;
    } while (--count);
    {
        struct B68 {
            CW_BITS2(unsigned char hi : 5, unsigned char lo : 3)
        };
        ((B68*)((ResolvedPreviewEquipment*)output)->appearance)->hi = hairStyle.colourIndex;
    }
    return 1;
}
#pragma use_lmw_stmw reset

#pragma opt_common_subs off
#pragma opt_propagation off
// Select a character and camera preset, rebuild saved equipment, and restore hair and ring choices.
int CModelOperationChara::ApplyCharacterSelection(void* key) {
    void* inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
    int characterIndexOrByteOffset = (int)ItemInventory_FindCharacterIndex(inventory, key);
    int valid = ModelOperationBase_ApplyCameraPreset(this, characterIndexOrByteOffset);
    if (valid == 0)
        return 0;
    char equipment[0x94];
    memset(equipment + 0x70, 0, 0x14);
    char attachmentIds[0x14];
    valid = BuildEquipmentFromSave(equipment, characterIndexOrByteOffset);
    if (valid != 0) {
        valid = ItemSelection_BuildOutfitFromRecord(&selection, equipment);
    } else {
        valid = 0;
    }
    if (valid != 0) {
        valid = ItemSelection_Validate(&selection);
        if (valid == 0)
            ItemSelection_RefreshPrimaryItems(&selection);
    }
    if (asyncAvatar == 0)
        return 0;
    Memory_Clear(attachmentIds, 0x14);
    characterIndexOrByteOffset = characterIndexOrByteOffset * 0x34;
    int auxSaveAddress = (int)saveData;
    int auxRecordAddress = auxSaveAddress + characterIndexOrByteOffset;
    signed char auxPartIndex =
        *(signed char*)(auxRecordAddress + IFF_OFFSET(PreviewSaveRecord, outfits[0].auxPartIndex));
    toggledItemIndex = auxPartIndex;
    *(int*)attachmentIds = (int)ItemInventory_GetAuxPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, auxPartIndex);
    int hairSaveAddress = (int)saveData;
    int hairRecordAddress = hairSaveAddress + characterIndexOrByteOffset;
    signed char hairStyleIndex =
        *(signed char*)(hairRecordAddress + IFF_OFFSET(PreviewSaveRecord, outfits[0].hairStyleIndex));
    selectedItemIndex = hairStyleIndex;
    return asyncAvatar->SetEquipment(&selection, attachmentIds);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Toggle caddie equipment or switch to its caddie, treating serial zero as an unequip request.
int CModelOperationCaddie::ToggleItem(unsigned int typeId) {
    unsigned int itemTypeId = typeId;
    if ((itemTypeId >> kItemKindShift) != kCaddieEquipmentItem || itemTypeId == 0)
        return 0;
    int caddieIndex = (itemTypeId >> kCaddieIndexShift) & kCaddieIndexMask;
    if (caddieIndex >= kCaddieCount)
        return 0;
    if (itemTypeId ==
        (unsigned int)ItemInventory_GetCaddieItemIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex)) {
        int resolvedItemOrCaddie =
            (int)ItemInventory_GetCaddieIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, caddieIndex);
        struct V {
            virtual void a() DECOMP_LOCAL_VIRTUAL;
            virtual void b() DECOMP_LOCAL_VIRTUAL;
            virtual void c() DECOMP_LOCAL_VIRTUAL;
            virtual int f(int) DECOMP_LOCAL_VIRTUAL;
        };
        return ((V*)this)->f(resolvedItemOrCaddie);
    }
    if ((itemTypeId & kCaddieItemSerialMask) == 0) {
        if (inventoryIndex == -1)
            return 0;
        ((PreviewSaveRecord*)saveRecord)->caddieItemIndex = -1;
        inventoryIndex = -1;
    } else {
        int resolvedItemOrCaddie =
            (int)ItemInventory_FindCaddieItemIndex(((CSharedDoc*)g_sharedDoc)->itemInventory, itemTypeId);
        ((PreviewSaveRecord*)saveRecord)->caddieItemIndex = (signed char)resolvedItemOrCaddie;
        inventoryIndex = (signed char)resolvedItemOrCaddie;
    }
    if ((*(int*)&asyncPet) == 0)
        return 0;
    if (ModelOperationBase_ApplyCameraPreset(this, caddieIndex) != 0) {
        return ((CAsyncExhibitionPet*)asyncPet)->SetPetSelection((void*)itemTypeId);
    }
    return 0;
}

#pragma inline_depth(0)
} // extern "C"
// Initialize the avatar preview operation and its empty part, hair and ring selection.
CModelOperationChara::CModelOperationChara() {
    ModelOperationBase_Ctor(this);
    asyncAvatar = 0;
    vtbl = __vt__20CModelOperationChara;
    ItemSelection_Init(&selection);
    selectedItemIndex = -1;
    toggledItemIndex = -1;
}
extern "C" {
#pragma inline_depth()

// Clear the pet receiver, destroy the base operation and optionally free the caddie operation.
CModelOperationCaddie::~CModelOperationCaddie() {
    (*(int*)&asyncPet) = 0;
    __dt__18ModelOperationBaseFv(this, 0);
}

// Clear the avatar receiver, destroy the base operation and optionally free the character operation.
CModelOperationChara::~CModelOperationChara() {
    asyncAvatar = 0;
    __dt__18ModelOperationBaseFv(this, 0);
}

// Apply caddie equipment only when the operation reports that its preview is ready.
int CModelOperationCaddie::TryToggleItem(int itemTypeId) {
    struct Vtbl {
        virtual void vfunc_0() DECOMP_LOCAL_VIRTUAL;
        virtual void vfunc_1() DECOMP_LOCAL_VIRTUAL;
        virtual int isPreviewReady() DECOMP_LOCAL_VIRTUAL;
    };
    if (((Vtbl*)this)->isPreviewReady() == 0) {
        return 0;
    }
    ToggleItem(itemTypeId);
    return 1;
}

#pragma inline_bottom_up on
#pragma inline_depth(255)
struct Obj802D9524 {
    int loadedBuffer;
    void* vtable;
    Obj802D9524(void* outer) {
        int r = CSharedFilePtr_GetLoadedBuffer((*(void**)&((CModelOperation*)outer)->sharedFile));
        loadedBuffer = 0;
        if (r != 0)
            loadedBuffer = r;
        vtable = (void*)__vt__21CExhibition_model_pos;
    }
};

// Create a position-table wrapper around the loaded shared-file buffer.
int ModelOperationBase_CreatePositionTable(void* self) {
    Obj802D9524* p = new Obj802D9524(self);
    (*(void**)&((CModelOperation*)self)->positionTable) = p;
    return 1;
}
#pragma inline_bottom_up off
#pragma inline_depth()

// Replace the current exhibition with a pet preview using the default viewport.
int CModelOperationCaddie::CreatePetPreview() {
    char viewport[16];
    if (ModelOperationBase_GetDefaultViewport(this, viewport) == 0) {
        return 0;
    }
    if (ModelOperationBase_ReleaseExhibition(this) == 0) {
        return 0;
    }
    void* pet = CAsyncExhibitionPet::Create(viewport, 1, kFloat_12_ModelPreview);
    asyncPet = pet;
    exhibition = pet;
    return pet != 0;
}

// Replace the current exhibition with an avatar preview using the default viewport.
int CModelOperationChara::CreateAvatarPreview() {
    char viewport[16];
    if (ModelOperationBase_GetDefaultViewport(this, viewport) == 0) {
        return 0;
    }
    if (ModelOperationBase_ReleaseExhibition(this) == 0) {
        return 0;
    }
    CAsyncExhibitionAvatar* avatar = CAsyncExhibitionAvatar::Create(viewport, 1, kFloat_10_ModelPreview);
    asyncAvatar = avatar;
    asyncExhibition = avatar;
    return avatar != 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462520", _ctors_u802D8A98_0, ModelOperation_InitFirstAnglePeriod);
STATIC_INIT_ENTRY(".CRT$XCU80462520", _ctors_u802D8A98_1, ModelOperation_InitSecondAnglePeriod);
#pragma pop

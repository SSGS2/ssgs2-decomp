#include <decomp/static_init.h>
#include <decomp/local_class.h>
typedef unsigned long new_size_t;
#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h"

enum PreviewItemKind {
    kPartItem = 2, kCaddieItem = 7, kCaddieEquipmentItem = 8,
    kHairStyleItem = 15, kAuxPartItem = 28, kItemKindShift = 26
};
enum PreviewItemIdBits {
    kCaddieIndexShift = 21, kCaddieIndexMask = 0x1F,
    kCaddieItemSerialMask = 0x1FFF, kCaddieCount = 8,
    kOutfitPartCount = 24, kNoInventoryIndex = -1
};
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
struct PreviewPartSelection { char prefix[0x24]; unsigned int partIds[kOutfitPartCount]; };
#define AsPreviewSave(p) ((PreviewSaveRecord*)(p))
#define AsPreviewEquipment(p) ((ResolvedPreviewEquipment*)(p))
#define AsCModelOperationCharaLayout(p) ((CModelOperationCharaLayout*)(p))
#include "classes/CModelOperationCharaLayout.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CExhibition.h"
#include "classes/CModelOperationCaddie.h"
#include "classes/CModelOperation.h"
#include "classes/CExhibition_model_pos.h"
#include <decomp/cw_bitfields.h>

#define AsCExhibition_model_pos(p) ((CExhibition_model_pos*)(p))

#define AsCModelOperation(p) ((CModelOperation*)(p))

#define AsCModelOperationCaddie(p) ((CModelOperationCaddie*)(p))

#define AsCExhibition(p) ((CExhibition*)(p))
// Translation unit 0x802D8A98-0x802D95DC (26 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" { void* __nw__FUl(unsigned long); void __dl__FPv(void*); }
inline void* operator new(new_size_t size) { return __nw__FUl(size); }
inline void* operator new(new_size_t, void* p) throw() { return p; }
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
int CModelOperationCaddie_SelectCaddie(void*, unsigned int);
void __dl__FPv(void*);
int CModelOperationCaddie_ToggleItem(void*, unsigned int);
int CModelOperation_OpenResource(void*, void*);
int ItemInventory_FindAuxPartIndex(void*, unsigned int);
unsigned char* ItemInventory_FindHairStyleData(void*, int);
int ItemInventory_FindHairStyleIndex(void*, unsigned int);
void CAsyncExhibition_ToggleAttachment(void*, int, unsigned int);
int ItemInventory_FindPartIndex(void*, int);
int CAsyncExhibition_CopyValidatedItemChanges(void*, void*, unsigned int);
int CAsyncExhibitionAvatar_RequestModelUpdate(void*, void*, int);
int SharedFile_Create(void*, int);
int CSharedFilePtr_GetLoadedBuffer(void*);
void ItemSelection_Init(void*);
extern const float kFloat_12_ModelPreview;
extern const float kFloat_10_ModelPreview;
extern char __vt__21CExhibition_model_pos[];
extern CSharedDoc* g_sharedDoc;
int ModelOperationBase_GetDefaultViewport(void*, void*);
int ModelOperationBase_ReleaseExhibition(void*);
void* CAsyncExhibitionPet_Create(void*, int, float);
void* CAsyncExhibitionAvatar_Create(void*, int, float);
int ItemInventory_FindCaddieIndex(void*, unsigned int);
int Memory_Clear(void*, int);
int ItemInventory_GetCharacterIdAt(void*, int);
int ItemInventory_GetPartIdAt(void*, int);
int ItemInventory_GetHairStyleIdAt(void*, int);
int ModelOperationBase_ApplyCameraPreset(void*, int);int CAsyncExhibitionPet_SetPetSelection(void*, unsigned int);
int CSharedFileHandle_Destroy(void*, int);
void* ItemInventory_FindCharacterIndex(void*, void*); /* (table, key): the original passes its caller's r4 through (0x802D8F78-0x802D8F80) */
void* ItemInventory_GetAuxPartIdAt(void*, int);
int ItemInventory_GetCaddieItemIdAt(void*, int);
int ItemInventory_GetCaddieIdAt(void*, int);
int ItemInventory_FindCaddieItemIndex(void*, unsigned int);
int ItemSelection_BuildOutfitFromRecord(void*, void*);
int ItemSelection_Validate(void*);
void ItemSelection_RefreshPrimaryItems(void*);
int CAsyncExhibitionAvatar_SetEquipment(void*, void*, void*);

// Open the shared character-preview resource through the base operation.
int CModelOperationChara_OpenResource(void* self) {
	CModelOperation_OpenResource(self, characterPreviewResourcePath);
	return 1;
}

// Reject non-part ids, then apply validated character part changes to the exhibition.
int CModelOperationChara_ApplyPartChanges(void* self, unsigned int partTypeId) {
	if ((partTypeId >> kItemKindShift) != kPartItem || partTypeId == 0) {
		return -1;
	}
	return CAsyncExhibition_CopyValidatedItemChanges(AsCModelOperationCharaLayout(self)->exhibition, ((char*)(AsCModelOperationCharaLayout(self)->operation)), partTypeId);
}

// Return whether the model operation's shared-file handle exposes a loaded buffer.
int ModelOperationBase_IsResourceLoaded(void* self) {
	int loadedBuffer = CSharedFilePtr_GetLoadedBuffer((*(void**)&AsCModelOperation(self)->sharedFile));
	switch (((unsigned int)(-loadedBuffer | loadedBuffer)) >> 31) {
		default: return 1;
		case 0: return 0;
	}
}

// Clear the caddie-equipment selection before applying the requested item.
void CModelOperationCaddie_SetItem(void* self, int typeId) {
	AsCModelOperationCaddie(self)->inventoryIndex = -1;
	CModelOperationCaddie_ToggleItem(self, typeId);
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
int ModelOperationBase_SetSaveRecord(void* self, int saveRecordAddress) {
	AsCModelOperation(self)->saveData = saveRecordAddress;
	return 1;
}

// Initialize the base operation with no resource, position table, exhibition or save record.
void ModelOperationBase_Ctor(void* self) {
	AsCModelOperation(self)->sharedFile = 0;
	AsCModelOperation(self)->vtbl = modelOperationBaseVtable;
	AsCModelOperation(self)->positionTable = 0;
	AsCModelOperation(self)->pointerRotationEnabled = 0;
	AsCModelOperation(self)->asyncExhibition = 0;
	AsCModelOperation(self)->saveData = 0;
}

// Open the shared caddie-preview resource through the base operation.
int CModelOperationCaddie_OpenResource(void* self) {
	CModelOperation_OpenResource(self, caddiePreviewResourcePath);
	return 1;
}

// Request an avatar model update unless the inventory index is invalid.
int CModelOperationChara_RequestModelUpdate(void* self, int modelIndex) {
	if (modelIndex == -1) {
		return 0;
	}
	CAsyncExhibitionAvatar_RequestModelUpdate(AsCModelOperationCharaLayout(self)->asyncAvatar, ((char*)(AsCModelOperationCharaLayout(self)->operation)), modelIndex);
	return 1;
}

// Toggle a ring inventory index and forward the attachment change to the avatar preview.
int CModelOperationChara_ToggleAttachment(void* self, int, unsigned int auxPartTypeId) {
	if ((auxPartTypeId >> kItemKindShift) != kAuxPartItem && auxPartTypeId != 0) {
		return 0;
	}
	int* inventory = (int*)(AsCSharedDoc(g_sharedDoc)->itemInventory);
	int auxPartIndex = ItemInventory_FindAuxPartIndex(inventory, auxPartTypeId);
	if (auxPartIndex == AsCModelOperationCharaLayout(self)->toggledItemIndex) {
		AsCModelOperationCharaLayout(self)->toggledItemIndex = -1;
	} else {
		AsCModelOperationCharaLayout(self)->toggledItemIndex = auxPartIndex;
	}
	CAsyncExhibition_ToggleAttachment(AsCModelOperationCharaLayout(self)->asyncAvatar, 0, auxPartTypeId);
	return 1;
}

// Resolve a hair-style id, remember its inventory index and apply its IFF colour index.
int CModelOperationChara_SetHairStyle(void* self, unsigned int hairStyleTypeId) {
	if ((hairStyleTypeId >> kItemKindShift) != kHairStyleItem || hairStyleTypeId == 0) {
		return 0;
	}
	unsigned char* hairStyleRecord = ItemInventory_FindHairStyleData(AsCSharedDoc(g_sharedDoc)->itemInventory, hairStyleTypeId);
	unsigned char colourIndex = ((IFF_STRUCT::sHairStyle*)hairStyleRecord)->colourIndex;
	int hairStyleIndex = ItemInventory_FindHairStyleIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, hairStyleTypeId);
	AsCModelOperationCharaLayout(self)->selectedItemIndex = hairStyleIndex;
	struct V { 
	virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual void c() DECOMP_LOCAL_VIRTUAL; virtual void call(void*, unsigned char) DECOMP_LOCAL_VIRTUAL;
	};
	((V*)AsCModelOperationCharaLayout(self)->exhibition)->call(((char*)(AsCModelOperationCharaLayout(self)->operation)), colourIndex);
}

#pragma use_lmw_stmw on
// Store preview part indices, hair and ring selection in the active character outfit.
int CModelOperationChara_WriteEquipmentToSave(void* self) {
	void* partCursor;
	int partByteOffset;
	int characterByteOffset;
	int partSlot;
	partCursor = self;
	partSlot = 0;
	partByteOffset = 0;
	characterByteOffset = AsSavePlayerRecord(AsCModelOperationCharaLayout(self)->saveRecord)->characterIndex * 0x34;
	for (; partSlot < kOutfitPartCount; ++partSlot) {
		short partIndex = (short)ItemInventory_FindPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, ((PreviewPartSelection*)partCursor)->partIds[0]);
		*(short*)((*(char**)&AsCModelOperationCharaLayout(self)->saveRecord) + characterByteOffset + partByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].partIndices)) = partIndex;
		partCursor = (char*)partCursor + 4;
		partByteOffset += 2;
	}
	int hairStyleIndex = AsCModelOperationCharaLayout(self)->selectedItemIndex;
	if (hairStyleIndex != -1) ((*(char**)&AsCModelOperationCharaLayout(self)->saveRecord))[characterByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].hairStyleIndex)] = (char)hairStyleIndex;
	((*(char**)&AsCModelOperationCharaLayout(self)->saveRecord))[characterByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].auxPartIndex)] = (char)AsCModelOperationCharaLayout(self)->toggledItemIndex;
	return 1;
}
#pragma use_lmw_stmw reset

// Create and remember a shared-file handle for the requested resource path.
int CModelOperation_OpenResource(void* self, void* path) {
	int sharedFile = SharedFile_Create(path, 0);
	AsCModelOperation(self)->sharedFile = sharedFile;
	return sharedFile != 0;
}

// Select a caddie, apply its camera preset, and clear its saved equipment selection.
int CModelOperationCaddie_SelectCaddie(void* self, unsigned int caddieTypeId) {
	if ((caddieTypeId >> kItemKindShift) != kCaddieItem || caddieTypeId == 0) return 0;
	void* inventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
	int caddieIndex = ItemInventory_FindCaddieIndex(inventory, caddieTypeId);
	if (AsCModelOperationCaddie(self)->asyncPet == 0) return 0;
	if (ModelOperationBase_ApplyCameraPreset(self, caddieIndex) == 0) return 0;
	AsCModelOperationCaddie(self)->inventoryIndex = -1;
	AsPreviewSave(AsCModelOperationCaddie(self)->saveRecord)->caddieItemIndex = -1;
	return CAsyncExhibitionPet_SetPetSelection(AsCModelOperationCaddie(self)->asyncPet, caddieTypeId);
}

// Release the exhibition, shared resource and position table, then optionally free the operation.
void* __dt__18ModelOperationBaseFv(void* self, int deleteStorage) {
	if (self != 0) {
		void* exhibition = (*(void**)&AsCModelOperation(self)->asyncExhibition);
		*(void**)self = modelOperationBaseVtable;
		if (exhibition != 0) {
			if (exhibition != 0) {
				((void (**)(void*, int))*(void**)exhibition)[2](exhibition, 1);
			}
			(*(void**)&AsCModelOperation(self)->asyncExhibition) = 0;
		}
		if ((*(void**)&AsCModelOperation(self)->sharedFile) != 0) {
			CSharedFileHandle_Destroy((*(void**)&AsCModelOperation(self)->sharedFile), 1);
			(*(void**)&AsCModelOperation(self)->sharedFile) = 0;
		}
		if ((*(void**)&AsCModelOperation(self)->positionTable) != 0) {
			if ((*(void**)&AsCModelOperation(self)->positionTable) != 0) {
				((void (**)(void*, int))(*(void**)((char*)(*(void**)&AsCModelOperation(self)->positionTable) + 4)))[2]((*(void**)&AsCModelOperation(self)->positionTable), 1);
			}
			(*(void**)&AsCModelOperation(self)->positionTable) = 0;
		}
		if (deleteStorage > 0) __dl__FPv(self);
	}
	return self;
}

#pragma use_lmw_stmw on
// Resolve saved part indices and hair colour into the character-preview equipment record.
int CModelOperationChara_BuildEquipmentFromSave(void* self, void* output, int characterIndexOrPartSlot) {
	char hairStyleBytes[0x64];
	int partByteOffset;
	void* partOutputCursor;
	int characterByteOffset;
	void* inventory;
	CSharedDoc* sharedDoc;
	int typeId;
	Memory_Clear(output, 0x94);
	sharedDoc = g_sharedDoc;
	inventory = AsCSharedDoc(sharedDoc)->itemInventory; // 0x802D92C0: inventory subobject at +0xA04
	if (inventory == 0) return 0;
	typeId = ItemInventory_GetCharacterIdAt(inventory, characterIndexOrPartSlot);
	AsPreviewEquipment(output)->characterIndex = characterIndexOrPartSlot;
	AsPreviewEquipment(output)->characterId = typeId;
	characterByteOffset = characterIndexOrPartSlot * 0x34;
	partOutputCursor = output;
	characterIndexOrPartSlot = 0;
	partByteOffset = 0;
	for (; characterIndexOrPartSlot < kOutfitPartCount; ++characterIndexOrPartSlot) {
		short partIndex = *(short*)((char*)AsCModelOperationCharaLayout(self)->saveRecord + characterByteOffset + partByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].partIndices));
		AsPreviewEquipment(partOutputCursor)->partIds[0] = ItemInventory_GetPartIdAt(inventory, partIndex);
		partByteOffset += 2;
		partOutputCursor = (char*)partOutputCursor + 4;
	}
	typeId = ItemInventory_GetHairStyleIdAt(inventory, (signed char)*((char*)AsCModelOperationCharaLayout(self)->saveRecord + characterByteOffset + IFF_OFFSET(PreviewSaveRecord, outfits[0].hairStyleIndex)));
	if (typeId == 0) return 0;
	unsigned int* src = (unsigned int*)ItemInventory_FindHairStyleData(inventory, typeId) - 1;
	unsigned int* dst = (unsigned int*)hairStyleBytes - 1;
	register int _cnt = 0x0C;
	do {
		unsigned int a = src[1];
		unsigned int b = src[2];
		src += 2;
		dst[1] = a;
		dst[2] = b;
		dst += 2;
	} while (--_cnt);
	{ struct B68 { CW_BITS2( unsigned char hi : 5, unsigned char lo : 3) }; ((B68*)AsPreviewEquipment(output)->appearance)->hi = *(unsigned char*)(hairStyleBytes + 0x5C); }
	return 1;
}
#pragma use_lmw_stmw reset

#pragma opt_common_subs off
#pragma opt_propagation off
// Select a character and camera preset, rebuild saved equipment, and restore hair and ring choices.
int CModelOperationChara_ApplyCharacterSelection(void* self, void* key) {
	void* inventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
	int characterIndexOrByteOffset = (int)ItemInventory_FindCharacterIndex(inventory, key); /* 0x802D8F80: r4 = the incoming key, not set here */
	int valid = ModelOperationBase_ApplyCameraPreset(self, characterIndexOrByteOffset);
	if (valid == 0) return 0;
	char equipment[0x94];
	memset(equipment + 0x70, 0, 0x14);
	char attachmentIds[0x14];
	valid = CModelOperationChara_BuildEquipmentFromSave(self, equipment, characterIndexOrByteOffset);
	if (valid != 0) {
		valid = ItemSelection_BuildOutfitFromRecord(((char*)(AsCModelOperationCharaLayout(self)->operation)), equipment);
	} else {
		valid = 0;
	}
	if (valid != 0) {
		valid = ItemSelection_Validate(((char*)(AsCModelOperationCharaLayout(self)->operation)));
		if (valid == 0) ItemSelection_RefreshPrimaryItems(((char*)(AsCModelOperationCharaLayout(self)->operation)));
	}
	if ((*(int*)&AsCModelOperationCharaLayout(self)->asyncAvatar) == 0) return 0;
	Memory_Clear(attachmentIds, 0x14);
	characterIndexOrByteOffset = characterIndexOrByteOffset * 0x34;
	int auxSaveAddress = (*(int*)&AsCModelOperationCharaLayout(self)->saveRecord);
	int auxRecordAddress = auxSaveAddress + characterIndexOrByteOffset;
	signed char auxPartIndex = *(signed char*)(auxRecordAddress + IFF_OFFSET(PreviewSaveRecord, outfits[0].auxPartIndex));
	int* selectedAuxPart = &AsCModelOperationCharaLayout(self)->toggledItemIndex; // CModelOperationChara_ApplyCharacterSelection: +0x88
	*selectedAuxPart = auxPartIndex;
	*(int*)attachmentIds = (int)ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, auxPartIndex);
	int hairSaveAddress = (*(int*)&AsCModelOperationCharaLayout(self)->saveRecord);
	int hairRecordAddress = hairSaveAddress + characterIndexOrByteOffset;
	signed char hairStyleIndex = *(signed char*)(hairRecordAddress + IFF_OFFSET(PreviewSaveRecord, outfits[0].hairStyleIndex));
	AsCModelOperationCharaLayout(self)->selectedItemIndex = hairStyleIndex;
	return CAsyncExhibitionAvatar_SetEquipment(AsCModelOperationCharaLayout(self)->asyncAvatar, ((char*)(AsCModelOperationCharaLayout(self)->operation)), attachmentIds);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Toggle caddie equipment or switch to its caddie, treating serial zero as an unequip request.
int CModelOperationCaddie_ToggleItem(void* self, unsigned int typeId) {
	unsigned int itemTypeId = typeId;
	if ((itemTypeId >> kItemKindShift) != kCaddieEquipmentItem || itemTypeId == 0) return 0;
	int caddieIndex = (itemTypeId >> kCaddieIndexShift) & kCaddieIndexMask;
	if (caddieIndex >= kCaddieCount) return 0;
	if (itemTypeId == (unsigned int)ItemInventory_GetCaddieItemIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCModelOperationCaddie(self)->inventoryIndex)) {
		int resolvedItemOrCaddie = (int)ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, caddieIndex);
		struct V { 
		virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual void c() DECOMP_LOCAL_VIRTUAL; virtual int f(int) DECOMP_LOCAL_VIRTUAL;
		};
		return ((V*)self)->f(resolvedItemOrCaddie);
	}
	if ((itemTypeId & kCaddieItemSerialMask) == 0) {
		if (AsCModelOperationCaddie(self)->inventoryIndex == -1) return 0;
		AsPreviewSave(AsCModelOperationCaddie(self)->saveRecord)->caddieItemIndex = -1;
		AsCModelOperationCaddie(self)->inventoryIndex = -1;
	} else {
		int resolvedItemOrCaddie = (int)ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, itemTypeId);
		AsPreviewSave(AsCModelOperationCaddie(self)->saveRecord)->caddieItemIndex = (signed char)resolvedItemOrCaddie;
		AsCModelOperationCaddie(self)->inventoryIndex = (signed char)resolvedItemOrCaddie;
	}
	if ((*(int*)&AsCModelOperationCaddie(self)->asyncPet) == 0) return 0;
	if (ModelOperationBase_ApplyCameraPreset(self, caddieIndex) != 0) {
		return CAsyncExhibitionPet_SetPetSelection(AsCModelOperationCaddie(self)->asyncPet, itemTypeId);
	}
	return 0;
}

#pragma inline_depth(0)
} // extern "C"
struct ModelOperationBase {
    char storage[0x1C];
    ModelOperationBase() { ModelOperationBase_Ctor(this); }
    ~ModelOperationBase();
};
struct CModelOperationChara : ModelOperationBase {
    char storage[0x70];
    CModelOperationChara();
};
// Initialize the avatar preview operation and its empty part, hair and ring selection.
CModelOperationChara::CModelOperationChara() {
    void* self = this;
	(*(int*)&AsCModelOperationCharaLayout(self)->asyncAvatar) = 0;
	AsCModelOperation(self)->vtbl = __vt__20CModelOperationChara;
	ItemSelection_Init(((char*)(AsCModelOperationCharaLayout(self)->operation)));
	AsCModelOperationCharaLayout(self)->selectedItemIndex = -1;
	AsCModelOperationCharaLayout(self)->toggledItemIndex = -1;
}
extern "C" {
#pragma inline_depth()

// Clear the pet receiver, destroy the base operation and optionally free the caddie operation.
void* CModelOperationCaddie_dtor(void* self, int deleteStorage) {
	if (self != 0) {
		(*(int*)&AsCModelOperationCaddie(self)->asyncPet) = 0;
		__dt__18ModelOperationBaseFv(self, 0);
		if (deleteStorage > 0) {
			__dl__FPv(self);
		}
	}
	return self;
}

// Clear the avatar receiver, destroy the base operation and optionally free the character operation.
void* CModelOperationChara_dtor(void* self, int deleteStorage) {
	if (self != 0) {
		(*(int*)&AsCModelOperationCharaLayout(self)->asyncAvatar) = 0;
		__dt__18ModelOperationBaseFv(self, 0);
		if (deleteStorage > 0) {
			__dl__FPv(self);
		}
	}
	return self;
}

// Apply caddie equipment only when the operation reports that its preview is ready.
int CModelOperationCaddie_TryToggleItem(void* self, int itemTypeId) {
	struct V { 
	virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual int e() DECOMP_LOCAL_VIRTUAL;
	};
	if (((V*)self)->e() == 0) {
		return 0;
	}
	CModelOperationCaddie_ToggleItem(self, itemTypeId);
	return 1;
}

#pragma inline_bottom_up on
#pragma inline_depth(255)
struct Obj802D9524 {
    int loadedBuffer;
    void* vtable;
    Obj802D9524(void* outer) {
        int r = CSharedFilePtr_GetLoadedBuffer((*(void**)&AsCModelOperation(outer)->sharedFile));
        loadedBuffer = 0;
        if (r != 0) loadedBuffer = r;
        vtable = (void*)__vt__21CExhibition_model_pos;
    }
};

// Create a position-table wrapper around the loaded shared-file buffer.
int ModelOperationBase_CreatePositionTable(void* self) {
    Obj802D9524* p = new Obj802D9524(self);
    (*(void**)&AsCModelOperation(self)->positionTable) = p;
    return 1;
}
#pragma inline_bottom_up off
#pragma inline_depth()

// Replace the current exhibition with a pet preview using the default viewport.
int CModelOperationCaddie_CreatePetPreview(void* self) {
    char viewport[16];
    if (ModelOperationBase_GetDefaultViewport(self, viewport) == 0) {
        return 0;
    }
    if (ModelOperationBase_ReleaseExhibition(self) == 0) {
        return 0;
    }
    void* pet = CAsyncExhibitionPet_Create(viewport, 1, kFloat_12_ModelPreview);
    AsCModelOperationCaddie(self)->asyncPet = pet;
    AsCModelOperationCaddie(self)->exhibition = pet;
    return pet != 0;
}

// Replace the current exhibition with an avatar preview using the default viewport.
int CModelOperationChara_CreateAvatarPreview(void* self) {
    char viewport[16];
    if (ModelOperationBase_GetDefaultViewport(self, viewport) == 0) {
        return 0;
    }
    if (ModelOperationBase_ReleaseExhibition(self) == 0) {
        return 0;
    }
    void* avatar = CAsyncExhibitionAvatar_Create(viewport, 1, kFloat_10_ModelPreview);
    AsCModelOperationCharaLayout(self)->asyncAvatar = avatar;
    AsCModelOperationCharaLayout(self)->exhibition = avatar;
    return avatar != 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462520", _ctors_u802D8A98_0, ModelOperation_InitFirstAnglePeriod);
STATIC_INIT_ENTRY(".CRT$XCU80462520", _ctors_u802D8A98_1, ModelOperation_InitSecondAnglePeriod);
#pragma pop

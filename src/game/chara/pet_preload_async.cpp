#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
// Byte offsets into the existing string pool; keep its storage and encoding.
enum {
    kCommonDialogStrings_common_dialogue_spd = 0x14, // common_dialogue.spd
    kCommonDialogStrings_black_sprite_anm_b2d = 0x28, // black_sprite_anm.b2d
};

// Byte offsets into the existing string pool; keep its storage and encoding.
enum {
    kPreloadPetAsyncStrings_ui_pet = 0x5, // _ui.pet
    kPreloadPetAsyncStrings_shw_pet = 0xd, // _shw.pet
    kPreloadPetAsyncStrings_basic_string_out_of_range = 0x61, // basic_string: out_of_range
    kPreloadPetAsyncStrings_std_exception_std_logic_error_std_out_of_range = 0x7c, // !std::exception!!std::logic_error!!std::out_of_range!!
};

// Pet preload contract: selection is a 32-bit IFF item ID carried in a pointer
// slot. The preload owns queued requests; inventory records and model-name bytes
// are borrowed while building them. A nonzero setup result means requests queued,
// not that loading succeeded. Poll marks ready when all owned requests complete.
// Shared dialogue resources form a separate requested -> ready lifecycle: request
// starts three file loads and seven texture loads, poll latches ready, release
// destroys both kinds of handles and clears both flags.
enum { kPetLoading = 0, kPetReady = 1, kCommonDialogTextureCount = 7, kItemKindShift = 26 };
enum PreloadItemKind { kPreloadClubSet = 4, kPreloadBall = 5, kPreloadCaddie = 7, kPreloadCaddieItem = 8 };
enum { kInlineStringLengthMask = 0x7f };
// Offsets in swingTaleStrings used by this vector specialization's exceptions.
enum VectorErrorStrings { kVectorLengthMessageOffset = 0x24, kLengthErrorRuntimeTypeOffset = 0x38 };
struct PetRecordVector { void* data; unsigned int size, capacity; };

#include "iff/IffStruct.h"
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12out_of_rangeFv(void*, int);
extern const char preloadPetAsyncStrings[];
}
}

#include "game/string.h"

// Translation unit 0x802789A8-0x80279C64 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CPreloadPetAsync.h"
#include "classes/CAsyncExhibition.h"
#include "classes/CAsyncExhibitionPet.h"

#define AsCPreloadPetAsync(p) ((CPreloadPetAsync*)(p))


extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u802789A8 {
    const unsigned int* words;
    bool active;
    // Guard a narrow/wide string heap buffer during exception unwinding.
    explicit StringUnwindCleanup_u802789A8(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disable unwind cleanup when normal string destruction takes over.
    void release() { active = false; }
    // Guard a narrow/wide string heap buffer during exception unwinding.
    ~StringUnwindCleanup_u802789A8() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};




namespace std {
class string;
class logic_error {
public:
	void* vtable;
	char* data;
	void* shared;
	~logic_error();
};
class out_of_range : public logic_error {
public:
	explicit out_of_range(const char*);
	explicit out_of_range(const string&);
	~out_of_range();
    DECOMP_CT_MEMBER(out_of_range)
};
class string {
public:
	explicit string(const char*);
	virtual ~string();
};
class length_error {
public:
	explicit length_error(const string&);
	virtual ~length_error();
};
}

extern void* CAsyncExhibition_GetExhibition(void* object);
extern void* SharedDoc_GetItemInventory();
extern void* ItemInventory_FindCaddieItemData(void* object, unsigned int value);
extern void* ItemInventory_FindCaddieData(void* object, unsigned int value);
extern void* ItemInventory_FindBallData(void* object, unsigned int value);
extern void* ItemInventory_FindClubSetData(void* object, unsigned int value);
extern void String_AssignCStringRange(void* object, void* arg);
extern void MWString_AppendCString(void* object, void* arg);
extern void* MWString_GetCString(void* object);
extern void* CPreloadWPuppetAsync_Create(void*);
extern void PreloadHandleVector_Append(void* object, void* arg);
extern void Storage_ClearThreeWords(void* object);

extern void* String_ReplaceRange(void* self, unsigned pos, unsigned cnt, const char* first, const char* last, void* extra);
extern void* __nwa__FUl(unsigned int size);
extern void* appMallocDummy__FUl(unsigned int size);
extern char preloadPetAsyncStrings[];
extern char __vt__Q23std12out_of_range[];
extern void __dt__Q23std12out_of_rangeFv();
extern char swingTaleStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern void CExhibition_SetItemType(void* object, int value);
extern void* CPreloadPetAsync_Create(void* object, int value);
extern int CPreloadPetAsync_ClearAndRestart(void* object, void* arg, int flag);
extern void CPreloadParent_dtor(void* object, int flag);
extern void OwnedObjectList_UpdateAll(void* object);
extern int OwnedObjectList_AreAllComplete(void* object);
extern void CPreloadParent_Ctor(void* object);
extern int PreloadObjectList_DestroyAll(void* object);
extern void CAsyncExhibition_dtor(void* object, int flag);
extern void __dl__FPv(void* object);
extern char __vt__16CPreloadPetAsync[];
extern char __vt__19CAsyncExhibitionPet[];
extern void appFreeUni__FPv(void* object);
extern void* CAsyncExhibition_UpdateLoadingStateAndIndicator(void* object, float delta);
extern void CAsyncExhibition_Ctor(void* object);
extern int CAsyncExhibition_TryInstallPreloadedObject(void* object);
extern void* SharedFile_Create(const char* name, int flag);
extern void CSharedFileHandle_Destroy(void* object, int flag);
extern void* CSharedFilePtr_GetLoadedBuffer(void* object);
extern void* __nw__FUl(unsigned int size);
extern void* CBpTex_Init(void* object);
extern char commonDialogStrings[];
// Seven texture handles and the corresponding texture-name pointers for shared dialogues.
extern void* s_commonDialogueTextures[];
extern void* s_commonDialogueTextureNames[];
extern void* s_commonDialogueLayoutFile; // common_dialogue.b2d handle
extern void* s_commonDialogueScriptFile; // common_dialogue.spd handle
extern void* s_commonDialogueBlackSpriteFile; // black_sprite_anm.b2d handle
extern int s_commonDialogueResourcesReady; // all shared dialogue resources are ready
extern int s_commonDialogueResourcesRequested; // shared dialogue resource loads have started

// Clear the vector size, release its allocation and optionally free the vector object.
void* PetRecordVector_dtor(void* object, int flag) {
	if (object != 0) {
		(*(void**)&((PetRecordVector*)object)->size) = 0;
		if (object != 0) {
			if (*reinterpret_cast<void**>(object) != 0) {
				(*(void**)&((PetRecordVector*)object)->size) = 0;
				appFreeUni__FPv(*reinterpret_cast<void**>(object));
			}
		}
		if (flag > 0) {
			__dl__FPv(object);
		}
	}
	return object;
}

// Update the exhibition and install a pending pet once its preload is ready.
int CAsyncExhibitionPet_UpdateAndInstallPendingPet(void* object, float delta) {
	if (CAsyncExhibition_UpdateLoadingStateAndIndicator(object, delta) == 0) {
		return 0;
	}
	unsigned char* pendingInstall = &AsCAsyncExhibitionPet(object)->updatePending; // CAsyncExhibitionPet_UpdateAndInstallPendingPet: +0x128
	if (*pendingInstall != 0) {
		void* child = ((CAsyncExhibition*)object)->preloadObject;
		if (child != 0) {
			struct V { 
			virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual int e() DECOMP_LOCAL_VIRTUAL;
			};
			if (reinterpret_cast<V*>(child)->e() != 0) {
				CAsyncExhibition_TryInstallPreloadedObject(object);
				*pendingInstall = 0;
			}
		}
	}
	return 1;
}

// Returns 0 for an unchanged ID, 1 for deferred installation, otherwise the install result.
// Change the pet selection immediately or defer installation while loading.
int CAsyncExhibitionPet_SetPetSelection(void* object, void* value) {
	void** field = &AsCAsyncExhibitionPet(object)->petSelection; // CAsyncExhibitionPet_SetPetSelection: +0x124
	if (*field == value) {
		return 0;
	}
	void* child = ((CAsyncExhibition*)object)->preloadObject;
	*field = value;
	if (child != 0) {
		struct V { 
		virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual int e() DECOMP_LOCAL_VIRTUAL;
		};
		if (reinterpret_cast<V*>(child)->e() == 0) {
			AsCAsyncExhibitionPet(object)->updatePending = 1;
			return 1;
		}
	}
	return CAsyncExhibition_TryInstallPreloadedObject(object);
}

// Returns object (r3 untouched, blr at 0x802789CC); CDrawDriveLine_UpdateShotPrediction uses it (0x80278260 etc.).
// Initialize an eight-word draw-drive-line record from seven values and a zero tail.
void* DriveLineRecord_Init(void* object, int value0, int value1, int value2, int value3,
				 int value4, int value5, int value6) {
	int* words = static_cast<int*>(object);
	words[0] = value0;
	words[1] = value1;
	words[2] = value2;
	words[3] = value3;
	words[4] = value4;
	words[5] = value5;
	words[6] = value6;
	words[7] = 0;
	return object;
}

// Report whether the pet preload is ready.
int CPreloadPetAsync_IsReady(void* object) {
	return AsCPreloadPetAsync(object)->loadState == kPetReady;
}

// Apply the selected item type to the exhibition model.
int CAsyncExhibitionPet_ApplyPetSelection(void* object) {
	void* result = CAsyncExhibition_GetExhibition(object);
	CExhibition_SetItemType(result, (int)AsCAsyncExhibitionPet(object)->petSelection);
	return 1;
}

extern float s_petUnitTwoPi0;
extern float s_petUnitTwoPi1;
extern float s_petUnitTwoPi2;
extern float kFloat_6_2831855;
extern float kFloat_0_AutoSetup;

// Clamp the value to a lower bound.
float Math_MaxFloat(float value, float limit) {
	if (value < limit) {
		value = limit;
	}
	return value;
}

// Clear the vector size using the original byte-distance expression.
void RecordVector_ClearSize(void* object) {
	*reinterpret_cast<int*>(static_cast<char*>(object) + 4) -= (static_cast<char*>(object) + *reinterpret_cast<int*>(static_cast<char*>(object) + 4)) - static_cast<char*>(object);
}

// Return the absolute value, preserving the original zero/NaN comparison behavior.
float Math_AbsFloatConditional(float value) {
	if (value > kFloat_0_AutoSetup) {
		return value;
	}
	return -value;
}

// Empty record cleanup hook.
void SixWordRecord_NoOpCleanup() {}

// Opaque 24-byte value payload; no word has established game semantics here.
// Vector layout is {data, size, capacity}, with counts in records and owned data.
enum SixWordVectorLimits { kSixWordMaxEntries = 0x0AAAAAAA,
    kSixWordFastGrowthLimit = 0x038E38E3, kSixWordHalfGrowthLimit = 0x071C71C6 };
struct SixWordRecord {
	unsigned a, b, c, d, e, f;
};

// Copy one record with the original volatile load order and exception boundary.
static inline void CopySixWordRecord(SixWordRecord* dst, const volatile SixWordRecord* source) {
		unsigned a = source->a;
		unsigned b = source->b;
		dst->a = a;
		unsigned c = source->c;
		dst->b = b;
		unsigned d = source->d;
		dst->c = c;
		unsigned e = source->e;
		dst->d = d;
		unsigned f = source->f;
		dst->e = e;
	try {
		dst->f = f;
	} catch (...) {
		throw;
	}
}

struct AllocatorTag {
	union { volatile char c; int pad; };
	// Store the allocator-tag byte required by the original template expansion.
	AllocatorTag() { c = 0; }
};

// New record storage during vector growth (name kept: it is part of the mangled destructor symbol).
struct Storage78A10 {
	SixWordRecord* data;
	unsigned size, capacity;
	unsigned* capacitySource;
	unsigned remaining;
	// Free the pending or swapped-out record allocation without separate payload cleanup.
	~Storage78A10() {
		size = 0;
		AllocatorTag flag;
		if (data != 0) {
			AllocatorTag inner;
			size -= size;
			appFreeUni__FPv(data);
		}
	}
};

extern char __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv();
extern void __dla__FPv(void*);

// Owns the error-text array until SharedErrorText takes it (name kept: mangled destructor symbol).
struct Buffer78A10 {
	char* data;
	void* allocator;
	// Free error-text array storage until ownership passes to SharedErrorText.
	~Buffer78A10() { if (data) __dla__FPv(data); }
};

struct SharedErrorText {
	void* vtable;
	unsigned owners, references;
	char* data;
	// Own an error-text array with initial strong/weak counts of one.
	SharedErrorText(char* buffer) {
		owners = 1;
		references = 1;
		vtable = s_sharedCharDeleterVtable;
		data = buffer;
	}
};



// Plain C: throw std::length_error("vector length error"); the text is copied into a
// reference-counted array so the exception owns it. Skip these blocks when reading.
// Build an owned length_error message and transfer it to the exception runtime.
// Keep this expansion in the caller's scope: extracting even a forced-inline
// function changes MWCC's stack slots and exception tables. EXTRA declares the
// caller's allocator tag; Buffer78A10 owns the text until SharedErrorText exists.
#define THROW_LENGTH_78A10(EXTRA) \
	{ \
		struct Error { void* vtable; char* data; SharedErrorText* shared; } error; \
		Error* object = &error; \
		EXTRA \
		const char* message = swingTaleStrings + kVectorLengthMessageOffset; \
		object->vtable = __vt__Q23std11logic_error; \
		AllocatorTag allocator; \
		unsigned length = strlen(message) + 1; \
		char* data = (char*)__nwa__FUl(length * 2 - length); \
		error.data = data; \
		{ \
		Buffer78A10 buffer = { data, &allocator }; \
		SharedErrorText* shared = new SharedErrorText(data); \
		error.shared = shared; \
		buffer.data = 0; \
		} \
		strcpy(error.data, message); \
		object->vtable = __vt__Q23std12length_error; \
		__throw(swingTaleStrings + kLengthErrorRuntimeTypeOffset, object, (void*)__dt__Q23std12length_errorFv); \
	}

// Append copies to pending storage, advancing its constructed-element count.
static inline void AppendSixWordRecords(Storage78A10& storage, SixWordRecord* dst,
		const SixWordRecord* source, unsigned n) {
	AllocatorTag flag;
	unsigned count = n;
	while (count != 0) {
		if (dst) CopySixWordRecord(dst, source);
		++storage.size;
		--count;
		++dst;
	}
}

// Move the old records backward into pending storage while tracking constructed entries.
static inline void RelocateSixWordRecords(Storage78A10& storage,
		const volatile SixWordRecord* begin, const volatile SixWordRecord* end) {
	SixWordRecord* dst = storage.data + storage.remaining;
	AllocatorTag relocateFlag;
	while (end > begin) {
		--dst;
		--end;
		if (dst) {
			try {
				unsigned a = end->a;
				unsigned b = end->b;
				dst->a = a;
				dst->b = b;
				dst->c = end->c;
				unsigned d = end->d;
				unsigned e = end->e;
				dst->d = d;
				dst->e = e;
				dst->f = end->f;
			} catch (...) {
				throw;
			}
		}
		--storage.remaining;
		++storage.size;
	}
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Choose capacity using a growth increment of at least needed and the 24-byte element limit.
static inline unsigned NextSixWordCapacity(unsigned capacity, const unsigned& needed) {
	if (capacity < kSixWordFastGrowthLimit) {
		unsigned increment = ((capacity + 1) * 4 - (capacity + 1)) / 5;
		const unsigned* selected = increment < needed ? &needed : &increment;
		return capacity + *selected;
	} else if (capacity < kSixWordHalfGrowthLimit) {
		unsigned increment = (capacity + 1) >> 1;
		const unsigned* selected = increment < needed ? &needed : &increment;
		return capacity + *selected;
	}
	return kSixWordMaxEntries;
}

// Append a record and swap in newly allocated storage when capacity is exhausted.
static inline void PushSixWordRecord(void* self, void* value) {
	struct Vector { SixWordRecord* data; unsigned size, capacity; };
	AllocatorTag topFlag;
	Vector* vector = (Vector*)self;
	SixWordRecord* source = (SixWordRecord*)value;
	unsigned size = vector->size;
	if (size < vector->capacity) {
		SixWordRecord* dst = vector->data + size;
		if (dst) CopySixWordRecord(dst, source);
		++vector->size;
		return;
	}
	AllocatorTag growFlag;
	AllocatorTag lengthFlag;
	if (kSixWordMaxEntries - vector->capacity < 1u) {
		THROW_LENGTH_78A10(AllocatorTag f1;);
	}
	// Phase 1: allocate guarded storage; the original vector is still intact.
	Storage78A10 storage = { 0, 0, 0, &vector->capacity, 0 };
	unsigned needed = vector->size + 1 - vector->capacity;
	unsigned capacity = vector->capacity;
	if (needed > kSixWordMaxEntries - capacity) {
		THROW_LENGTH_78A10(AllocatorTag f1;);
	}
	unsigned newCapacity;
	newCapacity = NextSixWordCapacity(capacity, needed);
	AllocatorTag allocateFlag;
	if (newCapacity > kSixWordMaxEntries) {
		THROW_LENGTH_78A10(AllocatorTag f1;);
	}
	storage.data = (SixWordRecord*)appMallocDummy__FUl(newCapacity * sizeof(SixWordRecord));
	unsigned oldSize = vector->size;
	storage.capacity = newCapacity;
	SixWordRecord* dst = storage.data + oldSize + storage.size;
	storage.remaining = oldSize;
	// Phase 2: construct the appended value first, allowing value to alias an old entry.
	AppendSixWordRecords(storage, dst, source, 1);
	RelocateSixWordRecords(storage, vector->data, vector->data + vector->size);
	// Phase 3: exchange allocations; storage now owns and frees the old buffer.
	vector->size = 0;
	SixWordRecord* oldData = vector->data;
	unsigned oldCapacity = vector->capacity;
	vector->capacity = storage.capacity;
	storage.capacity = oldCapacity;
	vector->data = storage.data;
	storage.data = oldData;
	vector->size = storage.size;
	storage.size = 0;
}
// Append one six-word record, growing the vector with exception-safe ownership transfer.
void SixWordRecordVector_PushBack(void* self, void* value) {
	PushSixWordRecord(self, value);
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma pop
#undef THROW_LENGTH_78A10

// Initialize the first local angle-wrap constant to one full turn.
void PetUnit_InitTwoPi0() {
	s_petUnitTwoPi0 = kFloat_6_2831855;
}

// Initialize the second local angle-wrap constant to one full turn.
void PetUnit_InitTwoPi1() {
	s_petUnitTwoPi1 = kFloat_6_2831855;
}

// Initialize the third local angle-wrap constant to one full turn.
void PetUnit_InitTwoPi2() {
	s_petUnitTwoPi2 = kFloat_6_2831855;
}

// Create a preload for the currently selected pet item.
void CAsyncExhibitionPet_CreatePetPreload(void* object) {
	CAsyncExhibitionPet* pet = AsCAsyncExhibitionPet(object);
	pet->petPreload = CPreloadPetAsync_Create(pet->petSelection, 1);
}

// Restart the existing pet preload when one is present.
int CAsyncExhibitionPet_RestartPetPreload(void* object) {
	CAsyncExhibitionPet* pet = AsCAsyncExhibitionPet(object);
	void* state = pet->petPreload;
	if (state == 0) {
		return 1;
	}
	CPreloadPetAsync_ClearAndRestart(state, pet->petSelection, 1);
	return 1;
}

// Destroy the preload parent and optionally free the object.
void* CPreloadPetAsync_dtor(void* object, int flag) {
	if (object != 0) {
		CPreloadParent_dtor(object, 0);
		if (flag > 0) {
			__dl__FPv(object);
		}
	}
	return object;
}

// Poll preload progress and mark the object ready after all requests complete.
int CPreloadPetAsync_UpdateAndMarkReady(void* object) {
	OwnedObjectList_UpdateAll(object);
	switch (AsCPreloadPetAsync(object)->loadState) {
	case 0:
		if (OwnedObjectList_AreAllComplete(object) != 0) {
			AsCPreloadPetAsync(object)->loadState = kPetReady;
		}
		break;
	default:
		break;
	}
	return 1;
	return 1;
}

// Destroy the exhibition base and optionally free the object.
void* CAsyncExhibitionPet_dtor(void* object, int flag) {
	if (object != 0) {
		CAsyncExhibition_dtor(object, 0);
		if (flag > 0) {
			__dl__FPv(object);
		}
	}
	return object;
}

// Search a narrow inline or heap string from the requested position; return -1 when absent.
int String_FindChar(void* object, signed char value, unsigned int position) {
	unsigned char* data;
	unsigned int length;
	if ((*reinterpret_cast<unsigned int*>(object) >> 31) == 0) {
		data = static_cast<unsigned char*>(object) + 1;
		length = *static_cast<unsigned char*>(object) & kInlineStringLengthMask;
	} else {
		data = (unsigned char*)((NString*)object)->data;
		length = ((NString*)object)->length;
	}
	if (position < length) {
		unsigned char* end = data + length;
		unsigned char* current = data + position;
		while (current < end) {
			if (value == static_cast<signed char>(*current)) {
				return current - data;
			}
			++current;
		}
	}
	return -1;
}

// Return the accessory-state block used to add equipment bonuses to player statistics.
void* CAccState_GetEquipmentBonuses(void* object) {
	return static_cast<char*>(object) + 0x10;
}

// Return whether a pet-selection update is pending.
unsigned char CAsyncExhibitionPet_vfunc_8(void* object) {
	return AsCAsyncExhibitionPet(object)->updatePending;
}

// Destroy prior preload objects, mark ready and dispatch a fresh initialization.
int CPreloadPetAsync_ClearAndRestart(void* object, void* arg, int flag) {
	int result;
	if (PreloadObjectList_DestroyAll(object) == 0) {
		result = 0;
	} else {
		AsCPreloadPetAsync(object)->loadState = kPetReady;
		result = 1;
	}
	if (result == 0) {
		return 0;
	}
	// Virtual slot 3 is the (re)initialization entry (see CPreloadPetAsync_Create's init call).
	struct PetPreloadVtable {
		virtual int slot0() DECOMP_LOCAL_VIRTUAL;
		virtual int slot1() DECOMP_LOCAL_VIRTUAL;
		virtual int slot2() DECOMP_LOCAL_VIRTUAL;
		virtual int init(void* arg, int flag) DECOMP_LOCAL_VIRTUAL;
	};
	return reinterpret_cast<PetPreloadVtable*>(object)->init(arg, flag);
}

// Allocate and initialize a pet preload, destroying it if initialization fails.
void* CPreloadPetAsync_Create(void* object, int arg) {
	struct Obj {
		// Initialize a ready pet preload with no queued requests.
		Obj() { CPreloadParent_Ctor(this); *(void**)this = __vt__16CPreloadPetAsync; AsCPreloadPetAsync(this)->loadState = kPetReady; }
		char unk[0x14];
	};
	Obj* p = new Obj;
	if (!p) return 0;
	struct VObj { 
	virtual void d2(int) DECOMP_LOCAL_VIRTUAL; virtual void d3() DECOMP_LOCAL_VIRTUAL; virtual void d4() DECOMP_LOCAL_VIRTUAL; virtual int init(void*, int) DECOMP_LOCAL_VIRTUAL;
	};
	VObj* vp = (VObj*)p;
	if (!vp->init(object, arg)) {
		if (p) vp->d2(1);
		return 0;
	}
	return p;
}

// Allocate and initialize a pet exhibition, destroying it if initialization fails.
void* CAsyncExhibitionPet_Create(void* object, void* arg, float value) {
	struct Obj {
		// Initialize the exhibition base and clear pet selection/loading state.
		Obj() { CAsyncExhibition_Ctor(this); *(void**)this = __vt__19CAsyncExhibitionPet; AsCAsyncExhibitionPet(this)->petSelection = 0; AsCAsyncExhibitionPet(this)->updatePending = 0; AsCAsyncExhibitionPet(this)->petPreload = 0; }
		char unk[0x130];
	};
	Obj* p = new Obj;
	if (!p) return 0;
	struct VObj { 
	virtual void d2(int) DECOMP_LOCAL_VIRTUAL; virtual void d3() DECOMP_LOCAL_VIRTUAL; virtual void d4() DECOMP_LOCAL_VIRTUAL; virtual void d5() DECOMP_LOCAL_VIRTUAL; virtual int init(float, void*, void*) DECOMP_LOCAL_VIRTUAL;
	};
	VObj* vp = (VObj*)p;
	if (!vp->init(value, object, arg)) {
		if (p) vp->d2(1);
		return 0;
	}
	return p;
}

#pragma push
#pragma use_lmw_stmw on
// Load the three shared dialogue files and initialize seven shared textures once.
void CommonDialogue_LoadSharedResources() {
	if (s_commonDialogueResourcesRequested != 0) {
		return;
	}
	s_commonDialogueLayoutFile = SharedFile_Create(commonDialogStrings, 0);
	s_commonDialogueScriptFile = SharedFile_Create(commonDialogStrings + kCommonDialogStrings_common_dialogue_spd, 0);
	s_commonDialogueBlackSpriteFile = SharedFile_Create(commonDialogStrings + kCommonDialogStrings_black_sprite_anm_b2d, 0);
	void** textureSlots = s_commonDialogueTextures;
	void** textureNames = s_commonDialogueTextureNames;
	for (int i = 0; i < kCommonDialogTextureCount; i++) {
		void* previousTexture = textureSlots[0];
		if (previousTexture != 0) {
			if (previousTexture != 0) {
				struct V { 
				virtual void c(int) DECOMP_LOCAL_VIRTUAL;
				};
				reinterpret_cast<V*>(previousTexture)->c(1);
			}
			textureSlots[0] = 0;
		}
		struct Obj {
			// Initialize the shared-dialogue texture wrapper.
			Obj() { CBpTex_Init(this); }
			char unk[0x28];
		};
		Obj* texture = new Obj;
		textureSlots[0] = texture;
		struct V { 
		virtual void a() DECOMP_LOCAL_VIRTUAL; virtual int b(void*) DECOMP_LOCAL_VIRTUAL;
		};
		reinterpret_cast<V*>(texture)->b(textureNames[0]);
		textureSlots++;
		textureNames++;
	}
	s_commonDialogueResourcesRequested = 1;
}
#pragma pop

// Release all shared dialogue files and textures and reset loading flags.
void CommonDialogue_ReleaseSharedResources() {
	if (s_commonDialogueResourcesRequested == 0) {
		return;
	}
	void* layoutFile = s_commonDialogueLayoutFile;
	if (layoutFile != 0) {
		CSharedFileHandle_Destroy(layoutFile, 1);
		s_commonDialogueLayoutFile = 0;
	}
	void* scriptFile = s_commonDialogueScriptFile;
	if (scriptFile != 0) {
		CSharedFileHandle_Destroy(scriptFile, 1);
		s_commonDialogueScriptFile = 0;
	}
	void* blackSpriteFile = s_commonDialogueBlackSpriteFile;
	if (blackSpriteFile != 0) {
		CSharedFileHandle_Destroy(blackSpriteFile, 1);
		s_commonDialogueBlackSpriteFile = 0;
	}
	void** textureSlots = s_commonDialogueTextures;
	int count = 0;
	void** textureSlot = textureSlots;
	void* zero = 0;
	for (unsigned int i = 0; i < kCommonDialogTextureCount; i++) {
		void* texture = textureSlot[0];
		if (texture != 0) {
			if (texture != 0) {
				struct V { 
				virtual void c(int) DECOMP_LOCAL_VIRTUAL;
				};
				reinterpret_cast<V*>(texture)->c(1);
			}
			textureSlot[0] = zero;
		}
		count++;
		textureSlot++;
	}
	s_commonDialogueResourcesRequested = 0;
	s_commonDialogueResourcesReady = 0;
}

// Wait for all three dialogue files and seven textures, then latch readiness.
int CommonDialogue_PollSharedResourcesLoaded() {
	if (s_commonDialogueResourcesReady != 0) {
		return 1;
	}
	if (s_commonDialogueResourcesRequested == 0) {
		return 0;
	}
	if (CSharedFilePtr_GetLoadedBuffer(s_commonDialogueLayoutFile) == 0) {
		return 0;
	}
	if (CSharedFilePtr_GetLoadedBuffer(s_commonDialogueScriptFile) == 0) {
		return 0;
	}
	if (CSharedFilePtr_GetLoadedBuffer(s_commonDialogueBlackSpriteFile) == 0) {
		return 0;
	}
	void** textureSlots = s_commonDialogueTextures;
	int count = 0;
	for (unsigned int i = 0; i < kCommonDialogTextureCount; i++) {
		void* texture = textureSlots[0];
		struct V { 
		virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void m1() DECOMP_LOCAL_VIRTUAL; virtual void m2() DECOMP_LOCAL_VIRTUAL; virtual void m3() DECOMP_LOCAL_VIRTUAL; virtual int m4() DECOMP_LOCAL_VIRTUAL;
		};
		if (reinterpret_cast<V*>(texture)->m4() == 0) {
			return 0;
		}
		count++;
		textureSlots++;
	}
	s_commonDialogueResourcesReady = 1;
	return 1;
}

#pragma push
#pragma use_lmw_stmw on
#pragma inline_max_size(1000)
#pragma inline_max_total_size(10000)
#pragma dont_inline off
// Construct the fixed substring-range error message and shared text ownership.
inline std::out_of_range::out_of_range(const char*) {
	AllocatorTag allocator;
	const char* message = preloadPetAsyncStrings + kPreloadPetAsyncStrings_basic_string_out_of_range;
	vtable = __vt__Q23std11logic_error;
	unsigned length = strlen(message) + 1;
	char* bufferData = (char*)__nwa__FUl(length * 2 - length);
	data = bufferData;
	{
		Buffer78A10 buffer = { bufferData, &allocator };
		SharedErrorText* sharedData = new SharedErrorText(bufferData);
		shared = sharedData;
		buffer.data = 0;
	}
	strcpy(data, message);
	vtable = __vt__Q23std12out_of_range;
}
// pos/count count bytes. count is clamped to the source tail; self==other is supported
// by the replacement routine. A start past the end throws std::out_of_range.
// Assign a bounded substring, rejecting a start beyond the source string length.
void String_AssignSubstring(void* self, void* other, unsigned pos, unsigned count) {
	unsigned selfLen;
	unsigned n = count;
	if ((*(unsigned int*)self >> 31) == 0) {
		selfLen = *(unsigned char*)self & kInlineStringLengthMask;
	} else {
		selfLen = ((NString*)self)->length;
	}
	unsigned otherLen;
	const char* otherData;
	if ((*(unsigned int*)other >> 31) == 0) {
		otherData = (const char*)other + 1;
		otherLen = *(unsigned char*)other & kInlineStringLengthMask;
	} else {
		otherData = (const char*)((NString*)other)->data;
		otherLen = ((NString*)other)->length;
	}
	if (pos > otherLen) {
		{
		    union { double alignment; unsigned char bytes[sizeof(std::out_of_range)]; } storage;
		    std::out_of_range* error = (std::out_of_range*)storage.bytes;
		    error->__ct(preloadPetAsyncStrings + kPreloadPetAsyncStrings_basic_string_out_of_range);
		    exception_runtime::__throw(exception_runtime::preloadPetAsyncStrings + kPreloadPetAsyncStrings_std_exception_std_logic_error_std_out_of_range, error,
		        (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
		}
	}
	unsigned remaining = otherLen - pos;
	otherData += pos;
	const unsigned* pcount = remaining < n ? &remaining : &n;
	n = *pcount;
	union { volatile char c; int align; } tmp;
	union { char c; int align; } extra;
	extra.c = tmp.c;
	String_ReplaceRange(self, 0, selfLen, otherData, otherData + n, &extra);
}
#pragma pop

#pragma push
#pragma dont_inline on
// Returns 0 for an unsupported/missing item or failed model-request creation.
// The main model request is retained even if the optional shadow cannot be queued;
// that case still returns 1. Shadow replaces everything from the FIRST '.' onward.
// Build the model path from an IFF item ID and queue its model and optional shadow preload.
int CPreloadPetAsync_QueueModelAndShadowFromItemId(void* self, unsigned itemId, int includeShadow) {
	struct Local {
		int w0, w1, w2;
	};
	Local modelPath;
	Storage_ClearThreeWords(&modelPath);
	StringUnwindCleanup_u802789A8 stringCleanup0(&modelPath);
	// Phase 1: select the IFF model/icon field and append the item-kind suffix.
	// The top 6 bits of an item id are its kind (caddie item, caddie, ball, club set, ...).
	switch (itemId >> kItemKindShift) {
	case kPreloadCaddieItem: {
		IFF_STRUCT::sCadItem* itemRecord = (IFF_STRUCT::sCadItem*)ItemInventory_FindCaddieItemData(SharedDoc_GetItemInventory(), itemId);
		if (itemRecord == 0) {
			{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
			return 0;
		}
		String_AssignCStringRange(&modelPath, itemRecord->model);
		String_AssignCStringRange(&modelPath, itemRecord->model);
		break;
	}
	case kPreloadCaddie: {
		IFF_STRUCT::sCaddie* itemRecord = (IFF_STRUCT::sCaddie*)ItemInventory_FindCaddieData(SharedDoc_GetItemInventory(), itemId);
		if (itemRecord == 0) {
			{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
			return 0;
		}
		String_AssignCStringRange(&modelPath, itemRecord->model);
		MWString_AppendCString(&modelPath, preloadPetAsyncStrings);
		break;
	}
	case kPreloadBall: {
		IFF_STRUCT::sBall* itemRecord = (IFF_STRUCT::sBall*)ItemInventory_FindBallData(SharedDoc_GetItemInventory(), itemId);
		if (itemRecord == 0) {
			{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
			return 0;
		}
		String_AssignCStringRange(&modelPath, itemRecord->model);
		MWString_AppendCString(&modelPath, preloadPetAsyncStrings);
		break;
	}
	case kPreloadClubSet: {
		IFF_STRUCT::sClubSet* itemRecord = (IFF_STRUCT::sClubSet*)ItemInventory_FindClubSetData(SharedDoc_GetItemInventory(), itemId);
		if (itemRecord == 0) {
			{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
			return 0;
		}
		String_AssignCStringRange(&modelPath, itemRecord->common.icon);
		MWString_AppendCString(&modelPath, preloadPetAsyncStrings + kPreloadPetAsyncStrings_ui_pet);
		break;
	}
	default:
		{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
		return 0;
	}
	// Phase 2: queue the required model before attempting the optional shadow.
	void* modelName = (void*)MWString_GetCString(&modelPath);
	void* preload = CPreloadWPuppetAsync_Create(modelName);
	if (preload == 0) {
		{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
		return 0;
	}
	PreloadHandleVector_Append((char*)self + IFF_OFFSET(CPreloadPetAsync, preloadRequests), &preload);
	// Phase 3: optionally queue a sibling shadow path; failure leaves the model request.
	if (includeShadow != 0) {
		int dot = String_FindChar(&modelPath, (signed char)'.', 0);
		if ((unsigned)dot != 0xffffffffu) {
			String_AssignSubstring(&modelPath, &modelPath, 0, (unsigned)dot);
			MWString_AppendCString(&modelPath, preloadPetAsyncStrings + kPreloadPetAsyncStrings_shw_pet);
			void* shadowName = (void*)MWString_GetCString(&modelPath);
			preload = CPreloadWPuppetAsync_Create(shadowName);
			if (preload != 0) {
				PreloadHandleVector_Append(&AsCPreloadPetAsync(self)->preloadRequests, &preload);
			}
		}
	}
	// Requests are owned by the preload list; discard only the temporary path here.
	AsCPreloadPetAsync(self)->loadState = kPetLoading;
	{ stringCleanup0.release(); ((NString*)(&modelPath))->~NString(); }
	return 1;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623B4", _ctors_u802789A8_0, PetUnit_InitTwoPi0);
STATIC_INIT_ENTRY(".CRT$XCU804623B4", _ctors_u802789A8_1, PetUnit_InitTwoPi1);
STATIC_INIT_ENTRY(".CRT$XCU804623B4", _ctors_u802789A8_2, PetUnit_InitTwoPi2);
#pragma pop

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CEnemyRenderRecord.h"
// Borrow the shared-document layout without changing ownership.
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char fontNumDrawStrings[];
}
}

// Translation unit 0x802534D8-0x80254BD4 (23 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Font/matrix registry infrastructure, enemy equipment transfer and round setup.
// Mod lookup: enemy appearance -> SharedDoc_ApplyEnemyRenderRecordToPlayerSlot;
// course/hole order and RNG -> CourseSelection_ApplyRoundSettings; registry
// storage -> vector helpers below.

#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

// Local views for storage not yet exposed by the shared class headers.
struct PointerArrayStorage { void** data; unsigned int size, capacity; };
struct ByteArrayStorage { unsigned char* data; unsigned int size, capacity; };
struct NumericDrawState { char prefix[0x24]; int pendingDraw; };
struct FontNumberPool { char prefix[11]; char vectorLengthError[20]; char lengthErrorType[55]; };
struct RoundSelection {
    unsigned char course, holeOrder, option, holeCount;
    ByteArrayStorage holes, courses;
};
struct SharedRoundArrays {
    char prefix[0x818]; unsigned char randomVariants[18];
    unsigned char holes[19]; unsigned char courses[19];
};
enum FontVectorLimits { kMaxFontVectorElements = 0x3FFFFFFF,
    kFontSmallGrowthLimit = 0x15555555, kFontHalfGrowthLimit = 0x2AAAAAAA,
    kFontRegistryLock = 3, kFontRegistryWriteLock = 1 };
enum RoundSelectionConstants { kCourseHoleCount = 18, kCourseHalfHoleCount = 9,
    kHoleVariantCount = 3, kHoleOrderFront = 0, kHoleOrderBack = 1, kHoleOrderShuffle = 3 };
// This round option still lies in padding in the shared CSharedDoc header.
struct SharedRoundOption { char prefix[0x9C6]; unsigned char option; };
enum EnemyRenderConstants { kPlayerSlotStride = 0x1D4,
    kPlayerPartCount = 24, kPlayerStatCount = 5, kPlayerItemCount = 8,
    kCharacterItemKind = 0x04000000, kCaddieKind = 0x1C000000 };
struct PlayerResourcePair { void* clubSet; void* ball; void* item; };
struct SharedPlayerEquipment {
    unsigned int typeId; int playerIndex; void* parts[24];
    unsigned char appearance; unsigned char statLevels[5]; char reserved6E[2];
    void* auxiliaryPart; int clearedControls[4]; unsigned char attributes[15];
};
struct SharedPlayerSlot {
    char prefix[0x84]; void* clubSet; void* ball; void* items[8];
    char beforeEquipment[0x74]; SharedPlayerEquipment equipment;
    int caddieSlot; int caddieId; int caddieItem;
    unsigned char caddieRank; char beforeClubCopy[11]; int clubSetCopy;
    short statValues[5]; char beforeFlags[2]; int renderFlag13; int renderFlag12;
};
struct EnemyRenderExtra {
    char prefix[0x18]; unsigned int playerItem;
    char beforeListValue[0x34]; unsigned int listValue;
    char beforeStats[0xC]; signed char statValues[5]; char reserved65[3];
    int itemIndices[8]; int caddieItem;
    unsigned char caddieRank; unsigned char attributes[14]; signed char statLevels[5];
};
// Transfer map, with offsets relative to the actual byte cursors used below:
// record+0x60: five signed stat values; +0x68: eight signed item indices;
// +0x8c: caddie rank; +0x8d..0x9a: fourteen uninterpreted bytes;
// +0x9b..0x9f: stat levels (negative = absent).
// equipment+0x84..0x91 receives those fourteen bytes in the same order;
// +0x92 receives levelIndex. No consumer here identifies individual byte meanings.
// These are copies of IDs/values, not transfers of inventory-record ownership.
typedef char SharedPlayerEquipmentSize[sizeof(SharedPlayerEquipment) == 0x94 ? 1 : -1];
typedef char SharedPlayerEquipmentOffset[(int)&((SharedPlayerSlot*)0)->equipment == 0x120 ? 1 : -1];

// Borrow an existing payload address for placement construction.
inline void *operator new(size_t, void *p) { return p; }

extern "C" {

extern float kFloat_6_2831855;
extern float s_fontNumDrawTau;
extern void *g_sharedDoc;
extern signed char s_bpSharedStartupGuard;
extern signed char s_bpSecondaryStartupGuard;
extern float s_textureCoordinateRegistryTau;

void Registry_DispatchVirtualOperation10(void *);
void *ostringstream_dtor(void *, short);
void TextureCoordinateRegistry_Dtor(void *, int);
void __dl__FPv(void *);
void *__nw__FUl(unsigned int);
void TableLock_ctor(void *, int, int);
void __dt__9TableLockFv(void *, int);
void UITextTable_Draw(void *);
void *TextureCoordinateRegistry_Init(void *, void *);
extern char __vt__26CTextureCoordinateRegistry[];
void PSMTX44Identity(void *);
void OStreamSentry_Ctor(void *, void *);
void MWStream_CopyLocale(void *, void *);
void MWLocale_InstallFacet(void *, void *, unsigned int);
void MWSharedOwner_ReleaseReference(void *);
void MWStream_ThrowFailure(void *, const char *);
void OStream_Flush(void *);
extern unsigned int s_streamLocaleFacetId;
extern unsigned int s_localeFacetIdCounter;
extern unsigned int s_roundPlayerList[];
extern char playerProfileDefaultNameStrings[];
void appFreeUni__FPv(void *);
void *appMallocDummy__FUl(unsigned int);
char *String_FormatTemporary(const char *, ...);
extern char iosClearFailureMessage[];
extern char stdNumPutCharVtable[];
extern char __vt__12CFontNumDraw[];
extern char __vt__12CFontNumDraw[];
extern char fontNumDrawStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern float kFloat_0_FontRenderer;
void *memcpy(void *, const void *, unsigned int);
void *memset(void *, int, unsigned int);
void *__nwa__FUl(unsigned int);
void __dla__FPv(void *);
void __ct__9CInfoBaseFv(void *);
void __dt__Q23std12length_errorFv(void *);
// The deleting destructor takes the flag in r4; a base subobject is destroyed with 0 (it never frees itself).
void __dt__9CInfoBaseFv(void *);
void FontNumDrawEntries_DestroyTree(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*);

struct TreeNode { TreeNode *left; TreeNode *right; void *unknown; unsigned int key; void *value; };
struct Tree { void *unknown; unsigned int count; unsigned int active; TreeNode sentinel; };
struct MatrixCopy { unsigned int words[16]; };

// Fill active indexed matrix slots with identity matrices, replacing entries found in the tree.
int Copy_IndexedMatricesOrIdentity(Tree *tree, void *output, unsigned int expectedCount) {
	if (!output) return 0;
	unsigned int count = tree->count;
	if (count != expectedCount) return 0;
	if (!tree->active) return 0;
	MatrixCopy *destination;
	TreeNode *sentinel;
	unsigned int i = 0;
	if (count != 0) {
		destination = (MatrixCopy *)output;
		sentinel = &tree->sentinel;
	for (; i < tree->count; ++destination, ++i) {
		PSMTX44Identity(destination);
		TreeNode *node = tree->sentinel.left;
		TreeNode *parent = &tree->sentinel;
		while (node) {
			if (node->key >= i) { parent = node; node = node->left; }
			else node = node->right;
		}
		if (parent == sentinel || i < parent->key) parent = &tree->sentinel;
		if (sentinel != parent) {
			MatrixCopy *source = (MatrixCopy *)parent->value;
			if (source) *destination = *source;
		}
	}
	}
	return 1;
}

// Metrowerks ostream ABI views (historical type spellings retained):
// OStreamHandle is the public ostream handle; data points to its ios/format state.
// OStreamState carries buffer, formatting and exception mask, not input bytes.
// NumPutFacet is the locale's num_put<char> facet; put is its numeric virtual hook.
// OutputIter wraps the output buffer/iterator word; zero denotes failed output.
// OStreamSentry is an ostream sentry (flushes unitbuf only on successful exit).
// LocaleRefGuard holds a copied locale reference; its destructor releases second.
// LocaleFacetTable is the locale facet table. Lookup returns its borrowed installed
// facet; the locale guard holds/releases a separate reference to locale storage.
struct OStreamHandle { void *data; };
struct OStreamState;
struct OutputIter {
	int value;
	// Wrap the output iterator/buffer word without acquiring ownership.
	OutputIter(int v) : value(v) {}
	// Test the ABI's null output-iterator failure representation.
	bool failed() const { return value == 0; }
};
struct NumPutFacet {
	virtual void vf0(); virtual void vf1(); virtual void vf2(); virtual void vf3();
	virtual void vf4(); virtual void vf5();
	unsigned int referenceCount;
	virtual OutputIter put(OutputIter, OStreamState *, signed char, int, float);
	// Forward numeric formatting using the iterator's buffer word as context.
	OutputIter format(const OutputIter &it, OStreamState *stream, signed char fill, float value) {
		return put(it, stream, fill, it.value, value);
	}
};

enum StreamStateBits { kStreamBad = 1, kStreamEof = 2, kStreamFail = 4, kStreamUnitBuffer = 0x2000 };
struct OStreamState {
	char pad00[0x24];
	int streamBuffer;
	char pad28[4];
	unsigned int formatFlags;
	unsigned short extendedFlags;
	unsigned char state;
	unsigned char exceptionMask;
	char pad34[4];
	signed char fillCharacter;
};

struct LocaleFacetTable {
	NumPutFacet **items;
	unsigned int count;
};

struct OStreamSentry {
	bool ok;
	bool failed;
	char pad[2];
	OStreamHandle *stream;
	// Construct the ostream sentry and record stream readiness.
	OStreamSentry(OStreamHandle *arg) { OStreamSentry_Ctor(this, arg); }
	// Flush unit-buffered output only when no sentry/stream failure is pending.
	~OStreamSentry() {
		OStreamState *s = (OStreamState *)stream->data;
		if (!(s->state & (kStreamBad | kStreamFail)) && (s->extendedFlags & kStreamUnitBuffer) && !failed) OStream_Flush(stream);
	}
};

struct LocaleRefGuard {
	LocaleFacetTable *table;
	void *second;
	// Retain a temporary copy of the stream's locale reference.
	LocaleRefGuard(void *arg) { MWStream_CopyLocale(this, arg); }
	// Release the temporary locale reference without throwing.
	~LocaleRefGuard() throw() { if (second) MWSharedOwner_ReleaseReference(second); }
};

// Borrow the locale's num_put facet, installing a reference-counted one if absent.
inline NumPutFacet *LookupNumPutFacet(LocaleRefGuard &lock) {
	if (!s_streamLocaleFacetId) {
		++s_localeFacetIdCounter;
		s_streamLocaleFacetId = s_localeFacetIdCounter;
	}
	unsigned int index = s_streamLocaleFacetId;
	NumPutFacet *existing;
	if (index >= lock.table->count || !(existing = lock.table->items[index])) {
		NumPutFacet *obj = (NumPutFacet *)__nw__FUl(8);
		if (obj) { obj->referenceCount = 0; *(void ***)obj = (void **)stdNumPutCharVtable; }
		LocaleFacetTable *table = lock.table;
		if (!s_streamLocaleFacetId) {
			++s_localeFacetIdCounter;
			s_streamLocaleFacetId = s_localeFacetIdCounter;
		}
		MWLocale_InstallFacet(table, obj, s_streamLocaleFacetId);
		existing = lock.table->items[index];
	}
	return existing;
}

#pragma use_lmw_stmw on
// Format a float through the locale facet and propagate stream errors using its exception mask.
void *OStream_InsertFloat(OStreamHandle *arg, float value) {
    // Phase 1: sentry checks readiness. Phase 2: hold locale only for facet
    // lookup; the installed facet remains locale-owned during format below.
	OStreamSentry guard(arg);
	try {
		if (guard.ok) {
			bool failed;
			try {
				NumPutFacet *obj;
				{
					LocaleRefGuard lock(arg->data);
					volatile bool dummy = false;
					obj = LookupNumPutFacet(lock);
				}
				OStreamState *source = (OStreamState *)arg->data;
				OutputIter result = obj->format(OutputIter(source->streamBuffer), source, source->fillCharacter, value);
				failed = result.failed();
			} catch (...) {
                // Phase 3: formatting exceptions set badbit; rethrow only if
                // masked. Failed output also adds failbit and may throw failure.
				OStreamState *err = (OStreamState *)arg->data;
				err->state |= kStreamBad;
				if (((OStreamState *)arg->data)->exceptionMask & kStreamBad) throw;
			}
			if (failed) {
				OStreamState *fail = (OStreamState *)arg->data;
				fail->state |= kStreamBad | kStreamFail;
				if (!fail->streamBuffer) fail->state |= kStreamBad;
				unsigned int mask = *(volatile unsigned char *)&fail->exceptionMask;
				if (fail->state & mask) MWStream_ThrowFailure(fail, iosClearFailureMessage);
			}
		}
	} catch (...) {
		guard.failed = true;
		throw;
	}
	return arg;
}
#pragma use_lmw_stmw reset

struct TableLock {
	int data[2];
	// Acquire the selected registry lock in the requested mode.
	TableLock(int a, int b) { TableLock_ctor(this, a, b); }
	// Release registry lock storage without deleting this guard.
	// The game's one ~TableLock is defined in wview_camera_projection.cpp (__dt__9TableLockFv); declaring it here without a body keeps
	~TableLock();
};

struct FontNumDrawBase {
	void *vtable;
	unsigned char pad[0x14];
	// Construct the numeric table's CInfoBase subobject.
	FontNumDrawBase() { __ct__9CInfoBaseFv(this); }
	// Unwind/destroy the numeric table's CInfoBase subobject.
	~FontNumDrawBase() { __dt__9CInfoBaseFv(this); }
};
struct FontRegistryLockGuard {
	int data[2];
	// Acquire the numeric table registry lock in the requested mode.
	FontRegistryLockGuard(int a, int b) { TableLock_ctor(this, a, b); }
	// Release the numeric table registry lock on normal or exceptional exit.
	~FontRegistryLockGuard() { __dt__9TableLockFv(this, -1); }
};
// Embedded container of the CFontNumDraw constructed as CFontNumDrawView below. Historical
// field spellings describe storage, not numeric glyph values: f18/f1c/f20/f24/f28
// are five words at owner+0x18..0x28. Construction makes f20 point to f1c,
// zeros f18/f1c/f24, and sets f28=1. f24 is the pending-draw word cleared by
// UITextTable_DrawList. Other word roles are unresolved; do not treat them as
// an array of drawable entries or assume that every zero is a null pointer.
struct FontNumDrawEntries {
	unsigned int f18;
	unsigned int f1c;
	unsigned int f20;
	unsigned int f24;
	unsigned int f28;
	// Defer member initialization to the enclosing numeric table constructor.
	FontNumDrawEntries() {}
	// Destroy the numeric table's embedded entry container.
	~FontNumDrawEntries() { FontNumDrawEntries_DestroyTree(this, -1); }
};
// CFontNumDrawView is the local construction view of CFontNumDraw (vtable evidence below),
// owned by the pointer vector. f2c/f30 are zero-initialized float parameters at
// +0x2c/+0x30; their units/uses are not established by this constructor.
struct CFontNumDrawView : FontNumDrawBase {
	FontNumDrawEntries member;
	float f2c;
	float f30;
	// Initialize the numeric table vtable, empty sentinel and two zero floats.
	CFontNumDrawView() {
		vtable = __vt__12CFontNumDraw;
		member.f18 = 0;
		member.f1c = 0;
		member.f20 = (unsigned int)&member.f1c;
		member.f24 = 0;
		member.f28 = 1;
		FontRegistryLockGuard inner(kFontRegistryLock, kFontRegistryWriteLock);
		float zero = kFloat_0_FontRenderer;
		f2c = zero;
		f30 = zero;
	}
};
struct PtrVector { CFontNumDrawView **data; unsigned int size; unsigned int capacity; };

// Borrow the larger of two live growth amounts (ties retain the first).
static unsigned int &umax(unsigned int &a, unsigned int &b) { return a < b ? b : a; }

struct StrRep { void *vt; unsigned int one; unsigned int one2; char *buf; };
struct MsgBase {
	void *vt;
	// Begin constructing an exception with the logic_error base vtable.
	MsgBase() { vt = __vt__Q23std11logic_error; }
	// Destroy the exception base during unwind or normal cleanup.
	~MsgBase() { __dt__Q23std9exceptionFv(this); }
};
struct CharGuard {
	char *buf;
	char *allocator;
	// Own newly allocated exception text until its shared holder takes over.
	CharGuard(char *p, char *a) : buf(p), allocator(a) {}
	// Free text only when its ownership was not released to a shared holder.
	~CharGuard() { if (buf) __dla__FPv(buf); }
};
struct MsgStr : MsgBase {
	char *buf;
	StrRep *rep;
	// Copy exception text into shared, reference-counted character storage.
	MsgStr(const char *msg, char *allocator) {
		unsigned int len = strlen(msg) + 1;
		buf = (char *)__nwa__FUl((unsigned int)((char *)(len * 2) - (char *)len));
		{
			CharGuard guard(buf, allocator);
			char *buffer = buf;
			StrRep *rp = new StrRep;
			if (rp) { rp->one = 1; rp->one2 = 1; rp->vt = s_sharedCharDeleterVtable; rp->buf = buffer; }
			rep = rp;
			guard.buf = 0;
		}
		strcpy(buf, msg);
	}
};
struct MsgAlloc {
    char value;
    // Initialize the exception string allocator marker.
    MsgAlloc() : value(0) {}
};
namespace std {
struct length_error : MsgStr {
	// Construct the shared message then install the length_error vtable.
	length_error(const char *msg, char *a) : MsgStr(msg, a) { vt = __vt__Q23std12length_error; }
	~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
struct VectorGuard_u802534D8 {
	CFontNumDrawView **data;
	unsigned int size;
	unsigned int capacity;
	unsigned int *capacityAddress;
	unsigned int offset;
	// Begin a pointer-buffer transaction and validate growth against max size.
	VectorGuard_u802534D8(unsigned int *a, unsigned int &cap, unsigned int extra) : data(0), size(0), capacity(0), capacityAddress(a), offset(0) {
		cap = *a;
		if (extra > kMaxFontVectorElements - cap) {
			MsgAlloc alloc;
			char *msg = ((FontNumberPool*)fontNumDrawStrings)->vectorLengthError;
			{
			    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
			    std::length_error* error = (std::length_error*)storage.bytes;
			    error->__ct(msg, &alloc.value);
			    exception_runtime::__throw(((const FontNumberPool*)exception_runtime::fontNumDrawStrings)->lengthErrorType, error,
			        (void*)exception_runtime::__dt__Q23std12length_errorFv);
			}
		}
	}
	// Free the uncommitted/old pointer buffer, without deleting pointed-to tables.
	~VectorGuard_u802534D8() { if (data) { size = 0; appFreeUni__FPv(data); } }
};

// Construct the vector length exception in aligned scratch storage and hand it to MW's throw ABI.
// The expansion's nested scopes preserve exception cleanup/code generation at each call site.
#define THROW_LEN() do { \
	MsgAlloc alloc; \
	char *msg = ((FontNumberPool*)fontNumDrawStrings)->vectorLengthError; \
	do { \
	    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage; \
	    std::length_error* error = (std::length_error*)storage.bytes; \
	    error->__ct(msg, &alloc.value); \
	    exception_runtime::__throw(((const FontNumberPool*)exception_runtime::fontNumDrawStrings)->lengthErrorType, error, \
	        (void*)exception_runtime::__dt__Q23std12length_errorFv); \
	} while (0); \
} while (0)

#pragma opt_propagation off
// Allocate a numeric text table and append it, growing the owning pointer vector if needed.
extern "C" CFontNumDrawView *CFontNumDraw_CreateInVector(PtrVector *tables) {
    // Phase 1: create a registry-owned table under the registry lock. Append
    // directly when capacity permits. Returned pointer is borrowed from tables.
    // Phase 2: otherwise reserve a new pointer buffer and place the new tail.
    // Phase 3: copy old pointers, clear old slots and swap buffers; the guard
    // frees the old buffer only, preserving ownership of the pointed-to tables.
	unsigned int threeFifthsGrowth;
	unsigned int halfGrowth;
	FontRegistryLockGuard lock(kFontRegistryLock, kFontRegistryWriteLock);
	CFontNumDrawView *item = new CFontNumDrawView;
	unsigned int size = tables->size;
	if (size < tables->capacity) {
		tables->size = tables->size + 1;
		tables->data[tables->size - 1] = item;
	} else {
	if ((unsigned int)(kMaxFontVectorElements - tables->capacity) < 1) {
		THROW_LEN();
	}
	unsigned int additionalCapacity = tables->size + 1 - tables->capacity;
	unsigned int newCapacity;
	VectorGuard_u802534D8 guard(&tables->capacity, newCapacity, additionalCapacity);
	if (newCapacity < kFontSmallGrowthLimit) {
		threeFifthsGrowth = (newCapacity + 1) * 3 / 5;
		newCapacity += umax(threeFifthsGrowth, additionalCapacity);
	} else if (newCapacity < kFontHalfGrowthLimit) {
		halfGrowth = (newCapacity + 1) / 2;
		newCapacity += umax(halfGrowth, additionalCapacity);
	} else {
		newCapacity = kMaxFontVectorElements;
	}
	if (newCapacity > kMaxFontVectorElements) THROW_LEN();
	guard.data = (CFontNumDrawView **)appMallocDummy__FUl(newCapacity * 4);
	guard.offset = tables->size;
	(guard.data + guard.offset)[guard.size] = item;
	guard.capacity = newCapacity;
	guard.size++;
	CFontNumDrawView **oldData = tables->data;
	int oldElementCount = ((int)((unsigned int)tables->data + tables->size * 4) - (int)oldData) / 4;
	guard.offset -= oldElementCount;
	unsigned int oldBytes = oldElementCount * 4;
	memcpy(guard.data + guard.offset, oldData, oldBytes);
	memset(oldData, 0, oldBytes);
	guard.size += oldElementCount;
	tables->size = 0;
	CFontNumDrawView **oldBuffer = tables->data;
	CFontNumDrawView **newBuffer = guard.data;
	guard.data = oldBuffer;
	unsigned int oldCapacity = tables->capacity;
	tables->capacity = guard.capacity;
	guard.capacity = oldCapacity;
	tables->data = newBuffer;
	tables->size = guard.size;
	guard.size = 0;
	}
	return item;
}
#pragma opt_propagation reset

struct VObject {
	void **vtable;
	char data[16];
	// Construct the texture-coordinate registry and install its derived vtable.
	VObject(void *arg) {
		TextureCoordinateRegistry_Init(this, arg);
		vtable = (void **)__vt__26CTextureCoordinateRegistry;
	}
};

// Draw each numeric text table under the registry lock and clear its pending-draw word.
int UITextTable_DrawList(void *arg) {
	TableLock lock(kFontRegistryLock, kFontRegistryWriteLock);
	void **cursor = ((PointerArrayStorage*)arg)->data;
	int zero = 0;
	while (cursor != ((PointerArrayStorage*)arg)->data + ((PointerArrayStorage*)arg)->size) {
		void *item = *cursor;
		UITextTable_Draw(item);
		((NumericDrawState*)item)->pendingDraw = zero;
		++cursor;
	}
	return 1;
}

class Q0 { public: 
virtual void v08(int); virtual int v0C();
};
// Construct a texture-coordinate registry and destroy it if initialization fails.
void *CTextureCoordinateRegistry_Create(void *arg) {
	VObject *obj = new VObject(arg);
	if (!obj)
		return 0;
	if (!((Q0*)obj)->v0C()) {
		if (obj) {
			((Q0*)obj)->v08(1);
		}
		return 0;
	}
	return obj;
}

// Find and destroy a matching pointer-array element, then shift the remaining entries.
int PointerArray_DeleteValue(void *arg, void *value) {
	TableLock lock(kFontRegistryLock, kFontRegistryWriteLock);
	void **iter = ((PointerArrayStorage*)arg)->data;
	while (iter != ((PointerArrayStorage*)arg)->data + ((PointerArrayStorage*)arg)->size && *iter != value)
		++iter;
	if (iter == ((PointerArrayStorage*)arg)->data + ((PointerArrayStorage*)arg)->size)
		return 0;
	if (*iter)
		((void (*)(void *, int))(*(void ***)*iter)[2])(*iter, 1);
	memmove(iter, iter + 1, (((PointerArrayStorage*)arg)->data + ((PointerArrayStorage*)arg)->size - iter - 1) * 4);
	((PointerArrayStorage*)arg)->size = ((PointerArrayStorage*)arg)->size - 1;
	return 1;
}

extern "C" void ByteVector_Clear(void *arg);

void *ItemInventory_GetBallIdAt(void *, int);
void *ItemInventory_GetClubSetIdAt(void *, int);
void *ItemInventory_GetHairStyleIdAt(void *, int);
void *ItemInventory_FindHairStyleData(void *, void *);
void *ItemInventory_GetAuxPartIdAt(void *, int);
void *ItemInventory_GetPartIdAt(void *, short);
int ItemInventory_GetCaddieItemIdAt(void *, int);
void *ItemInventory_GetItemIdAt(void *, int);

typedef IffRecordStorage<IFF_STRUCT::sHairStyle> HairStyleCopy;
struct Flags68 { CW_BITS2( unsigned char high : 5, unsigned char low : 3) };
// Wii bitfield layout: appearance high five bits = hairstyle colourIndex;
// low three = baseTextureVariant truncated to 3 bits. Producer is the enemy
// render record (FillFromSaveData/BuildItemSelection in character_preview_avatars.cpp), then this
// routine packs into the live player descriptor at doc+slot*0x1D4+0x188.
// Its consumer is character/model equipment selection, not a color float.
// Source flags bits 13/12 are copied as separate int 0/1 at slot+0x1CC/+0x1D0;
// this transfer does not interpret them. The save-data producer maps source
// previewOptionBits bits30/31 into record mask0x3000; record bit14 is the
// independent preview-name dirty bit (NOT copied).
// Fourteen attributes remain opaque byte copies; see offset transfer map above.
// Keep these flags/attributes with the same equipment snapshot. Their concrete
// gameplay effects require consumer research before changing their values.

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
// Resolve enemy inventory indices and copy appearance, equipment, stats and flags into a player slot.
void SharedDoc_ApplyEnemyRenderRecordToPlayerSlot(int index, const void *records) {
    // index must address both a writable shared-player slot and a live render
    // record. records is the render-table indexing origin, not a separately
    // advanced record pointer. Inventory lookups borrow the global inventory.
    // Cursor origins: playerSlotBytes=doc+index*0x1d4; equipmentBytes=slot+0x120;
    // renderRecordBytes=records+index*sizeof(CEnemyRenderRecord).
    // Preconditions: index is a valid player slot and records contains that
    // many 0xa4-byte render records; all referenced inventory tables are ready.
    // Input is borrowed. Output is the strided g_sharedDoc player slot; values
    // returned by inventory accessors are ID words, even where typed as void*.
    // Phase 1: unpack record flag bits 13/12 unchanged (gameplay meaning remains
    // unresolved), resolve ball/club and encode the character kind in typeId.
	unsigned short flags;
	int renderFlag13;
	int renderFlag12;
	unsigned int playerSlotOffset;
	int renderRecordOffset;
	char *playerSlotBytes;
	char *equipmentBytes;
	char *resourceCursor;
	const char *renderRecordBytes;
	const char *itemIndexCursor;
	const char *partIndexCursor;
	char *partIdCursor;
	int i;
	int j;
	playerSlotOffset = index * kPlayerSlotStride;
	renderRecordOffset = index * sizeof(CEnemyRenderRecord);
	renderRecordBytes = (const char *)records + renderRecordOffset;
	const CEnemyRenderRecord *rec = (const CEnemyRenderRecord *)renderRecordBytes; // source record (enemy render table)
	flags = rec->flags;
	renderFlag13 = (flags >> 13) & 1;
	renderFlag12 = (flags >> 12) & 1;
	int ballIndex = rec->ballInventoryIndex;
	playerSlotBytes = (char *)g_sharedDoc + playerSlotOffset;
	SharedPlayerSlot *slot = (SharedPlayerSlot *)playerSlotBytes; // destination player slot
	slot->renderFlag13 = renderFlag13;
	equipmentBytes = (char*)&slot->equipment;
	SharedPlayerEquipment *equip = (SharedPlayerEquipment *)equipmentBytes; // slot->equipment
	resourceCursor = (char*)&slot->clubSet;
	slot->renderFlag12 = renderFlag12;
	((PlayerResourcePair*)resourceCursor)->ball = ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), ballIndex);
	int clubSetIndex = rec->clubSetInventoryIndex;
	void *clubSetId = ItemInventory_GetClubSetIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), clubSetIndex);
	unsigned int recordId = rec->characterInventoryIndex;
	((PlayerResourcePair*)resourceCursor)->clubSet = clubSetId;
	equip->typeId = recordId | kCharacterItemKind;
	equip->playerIndex = index;
	int hairStyleIndex = rec->hairStyleIndex;
	void *hairStyleId = ItemInventory_GetHairStyleIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), hairStyleIndex);
	char *hairStyleData = (char *)ItemInventory_FindHairStyleData(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), hairStyleId);
	HairStyleCopy hairStyle = *(HairStyleCopy *)hairStyleData;
	((Flags68 *)&equip->appearance)->high = hairStyle.value.colourIndex;
	((Flags68 *)&equip->appearance)->low = rec->baseTextureVariant;
	equip->auxiliaryPart = ItemInventory_GetAuxPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), rec->auxiliaryPartInventoryIndex);
	equip->clearedControls[0] = 0;
	equip->clearedControls[1] = 0;
	equip->clearedControls[2] = 0;
	equip->clearedControls[3] = 0;
	partIndexCursor = renderRecordBytes;
    // Phase 2: 24 signed-short inventory indices -> 24 four-byte part IDs.
    // These byte cursors intentionally begin at enclosing objects, then add the
    // field offset each iteration; they are NOT pointers to independent records.
	partIdCursor = equipmentBytes;
	for (i = 0; i < kPlayerPartCount; ++i) {
		*(void **)(partIdCursor + IFF_OFFSET(SharedPlayerEquipment, parts)) = ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), *(short *)(partIndexCursor + IFF_OFFSET(CEnemyRenderRecord, partInventoryIndices)));
		partIndexCursor += 2;
		partIdCursor += 4;
	}
	slot->caddieSlot = index;
	// Encode the caddie inventory index with item kind seven; negative means absent.
	int caddieIndex = rec->caddieInventoryIndex;
	if (caddieIndex < 0) slot->caddieId = 0;
	else slot->caddieId = caddieIndex | kCaddieKind;
	slot->clubSetCopy = (*(int*)&((PlayerResourcePair*)resourceCursor)->clubSet);
    // Phase 3a: copy five stat values, testing the corresponding signed level.
	for (int i = 0; i < kPlayerStatCount; ++i) {
        // Phase 3: negative level is the absent-stat marker; store zero for
        // both value and level. Nonnegative levels preserve signed stat values.
		signed char statLevel = *(signed char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, statLevels) + i);
		if (statLevel >= 0) *(short *)(playerSlotBytes + IFF_OFFSET(SharedPlayerSlot, statValues) + i * (int)sizeof(short)) = *(signed char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, statValues) + i);
		else *(short *)(playerSlotBytes + IFF_OFFSET(SharedPlayerSlot, statValues) + i * (int)sizeof(short)) = 0;
	}
    // Phase 3b: repeat the same validity test for five equipment-level bytes.
	for (int i = 0; i < kPlayerStatCount; ++i) {
		signed char statLevel = *(signed char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, statLevels) + i);
		if (statLevel >= 0) *(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, statLevels) + i) = (unsigned char)statLevel;
		else *(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, statLevels) + i) = 0;
	}
	slot->caddieItem = ItemInventory_GetCaddieItemIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), rec->caddieItemInventoryIndex);
	slot->caddieRank = ((const EnemyRenderExtra*)renderRecordBytes)->caddieRank;
	itemIndexCursor = renderRecordBytes;
    // Phase 4: resourceCursor still starts at slot.clubSet. The `item` field
    // of its three-word view is +8, hence each +4 cursor step writes the next
    // slot.items[j], not a new PlayerResourcePair. Negative item indices clear.
	for (j = 0; j < kPlayerItemCount; ++j) {
		((PlayerResourcePair*)resourceCursor)->item = 0;
		int id = *(int *)(itemIndexCursor + IFF_OFFSET(EnemyRenderExtra, itemIndices));
		if (id >= 0) ((PlayerResourcePair*)resourceCursor)->item = ItemInventory_GetItemIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), id);
		resourceCursor += 4;
		itemIndexCursor += 4;
	}
	unsigned char attributeSecond, attributeThird, attributeFirst;
    // Phase 5: byte-for-byte transfer of 14 unresolved appearance/options
    // attributes; destination attribute[14] additionally stores levelIndex.
    // The first/second/third temporaries preserve load-before-store ordering.
    // Do not assign gameplay names to these slots without tracing consumers.
	attributeFirst = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 0);
	attributeSecond = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 1);
	attributeThird = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 2);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 0) = attributeFirst;
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 1) = attributeSecond;
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 2) = attributeThird;
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 3) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 3);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 4) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 4);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 5) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 5);
	attributeThird = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 6);
	attributeFirst = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 7);
	attributeSecond = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 8);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 6) = attributeThird;
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 7) = attributeFirst;
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 8) = attributeSecond;
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 9) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 9);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 10) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 10);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 11) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 11);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 12) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 12);
	*(unsigned char *)(equipmentBytes + IFF_OFFSET(SharedPlayerEquipment, attributes) + 13) = *(unsigned char *)(renderRecordBytes + IFF_OFFSET(EnemyRenderExtra, attributes) + 13);
	equip->attributes[14] = rec->levelIndex;
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
#pragma opt_lifetimes reset

// Initialize the registry angle constant and mark both BP startup guards initialized.
void TextureCoordinateRegistry_InitAngleAndBpGuards() {
	s_textureCoordinateRegistryTau = kFloat_6_2831855;
	if (!s_bpSharedStartupGuard)
		s_bpSharedStartupGuard = 1;
	if (!s_bpSecondaryStartupGuard)
		s_bpSecondaryStartupGuard = 1;
}

// Destroy a texture-coordinate registry and optionally free its object storage.
void *CTextureCoordinateRegistry_dtor(void *arg, int flag) {
	if (arg) {
		TextureCoordinateRegistry_Dtor(arg, 0);
		if (flag > 0)
			__dl__FPv(arg);
	}
	return arg;
}

// Adjust a secondary-interface pointer to its owner before calling the owner destructor.
void *TextureCoordinateRegistry_DestructOwnerFromInterface(char *arg, short deleting) {
	unsigned int offset = *(unsigned int *)(arg + 0x3C);
	return ostringstream_dtor(arg + offset - 0x40, deleting);
}

extern "C" void Struct12_Clear(void *arg);

// Dispatch registry operation ten through its shared virtual interface.
void CTextureCoordinateRegistry_vfunc_1(void *arg) {
	Registry_DispatchVirtualOperation10(arg);
}

// Round-player list ABI: global header is {count, prev, next}; sentinel=head+1.
// Nodes own 8 bytes of links followed by this 0x6c-byte value. Payload offsets:
//   +00 player index; +04/+1a two 22-byte default-name copies; +30 20 bytes
//   untouched here; +44 byte copy of index; +48 playerItem; +4c listValue;
//   +50 enabled (record flags bit15); +54 default flag word 0x30000000;
//   +58 zero; +5c four option bytes; +60/+64/+68 trailing words.
// opt0a is byte0's high two bits; opt1b byte1's middle six bits (mask 0x7e).
// Their meanings and the third name buffer's role are unresolved. Constructor
// initializes only named assignments below; it does NOT zero all payload bytes.
// name1/name2 receive identical bounded strncpy copies (not guaranteed NUL if
// truncated). No evidence here distinguishes display/account/player name roles.
// Editing guide: options and bits are two views of the SAME four bytes. On the
// target's big-endian bitfield ABI, bits.opt0a=options[0]&0xc0 (shifted),
// opt0b=options[0]&0x3f, opt1a=byte1 bit7, opt1b=byte1 bits6..1, opt1c=bit0.
// The default constructor clears all of byte0 but only bits6..1 of byte1;
// byte1 bits7/0, options[2/3], tail0/tail1 and name3 are left untouched.
// tail2 alone is initialized to zero. Copy construction preserves the whole
// value, including untouched bytes. Do not replace it with a zeroed value.
struct ListPayload {
 unsigned int index;
 char name1[0x16];
 char name2[0x16];
 char name3[0x14];
 unsigned char playerIndexByte;
 unsigned int item, value, enabled, flag, zero;
 union {
  unsigned char options[4];
  struct {
   CW_BITS2(unsigned char opt0a:2, unsigned char opt0b:6)   // CodeWarrior order: opt0a = 0xC0, opt0b = 0x3F
   CW_BITS3(unsigned char opt1a:1, unsigned char opt1b:6, unsigned char opt1c:1)   // opt1a = 0x80, opt1b = 0x7E, opt1c = 0x01
   unsigned char opt2, opt3;
  } bits;
 };
 unsigned int tail0, tail1, tail2;
 // Initialize only the established default fields, leaving unresolved bytes alone.
 ListPayload() {
   tail2 = 0; index = 0; bits.opt0a = 0; bits.opt1b = 0; zero = 0; playerIndexByte = 0;
  bits.opt0b = 0; enabled = 0; flag = 0x30000000; item = 0;
 }
 // Copy the full inline player-list payload, including unresolved bytes.
 ListPayload(const ListPayload &other) { try { *this = other; } catch (...) { throw; } }
 // Store the list entry's enabled state as a word.
 void setEnabled(bool v) { enabled = v; }
 // Set player index/item/value/enabled and reset the high two option bits.
 void set(unsigned int id, unsigned int it, unsigned int val, int e) {
   playerIndexByte = id; enabled = (bool)e; bits.opt0a = 0; index = id; item = it; value = val;
 }
};
struct ListAlloc {
 volatile char value;
 // Initialize the list allocator marker.
 ListAlloc() : value(0) {}
 // Preserve the original allocator-copy no-op.
 ListAlloc(const ListAlloc &) {}
};
struct ListRecordFlags { CW_BITS2(unsigned short enabled:1, unsigned short rest:15) };
struct ListRawNode {
 unsigned int links[2];
 char payload[0x6C];
 // Allocate an owned node from the application allocator.
 static void *operator new(size_t n) { return appMallocDummy__FUl(n); }
 // Release a node whose construction failed or ownership ended.
 static void operator delete(void *p) { appFreeUni__FPv(p); }
 // Leave node links/payload for the guarded placement-construction phase.
 ListRawNode() {}
};
struct ListNodeGuard {
 unsigned int *allocation;
 ListAlloc allocator;
 unsigned int *position;
 unsigned int *node;
 // Own the newly allocated node until its payload can be linked into the list.
 ListNodeGuard(unsigned int *p, const ListAlloc &a, unsigned int *s) : allocation(p), allocator(a), position(s), node(p) {}
 // Free an uncommitted node; committed nodes clear node before guard exit.
 ~ListNodeGuard() { if (node) appFreeUni__FPv(node); }
};
// Unlink and free all owned list nodes; embedded sentinel and global header survive.
inline void clearList(unsigned int *head) {
 // Word-index legend used by both list routines: head[0]=count;
 // head[1]/sentinel[0]=prev, head[2]/sentinel[1]=next.
 // For allocated nodes: node[0]=prev, node[1]=next, node+2=inline payload.
 // Integer link words carry 32-bit addresses; they are not player/item IDs.
 unsigned int *sentinel = head + 1;
 unsigned int *node = (unsigned int *)head[2];
 if (node != sentinel) {
  unsigned int *last = (unsigned int *)sentinel[0];
  unsigned int *first = (unsigned int *)node[0];
  first[1] = last[1];
  ((unsigned int *)last[1])[0] = node[0];
  while (node != sentinel) {
   unsigned int *current = node;
   node = (unsigned int *)node[1];
   appFreeUni__FPv(current);
   --head[0];
  }
 }
}

#pragma use_lmw_stmw on
// Replace the round player list with resolved render-table entries and default player names.
void GolfPlayerList_BuildFromRenderTable(void *renderTable) {
    // Phase 1: discard old nodes. Phase 2: resolve each player into shared state,
    // build the list value and two default names, then copy into a guarded node.
    // Phase 3: release the guard only after construction, append before sentinel,
    // increment count. The list owns nodes; renderTable remains borrowed.
 char *recordCursor;
 unsigned int *sentinel;
 unsigned int *head;
 int playerIndex;
 unsigned int value;
 unsigned int item;
 int enabled;
 ListPayload playerEntry;
	head = s_roundPlayerList;
 clearList(head);
 playerIndex = 0;
 if (((CEnemyRenderTable*)renderTable)->count > 0) {
 recordCursor = (char *)renderTable;
 sentinel = head + 1;
 for (; playerIndex < ((CEnemyRenderTable*)renderTable)->count; recordCursor += sizeof(CEnemyRenderRecord), ++playerIndex) {
		SharedDoc_ApplyEnemyRenderRecordToPlayerSlot(playerIndex, renderTable);
  enabled = ((ListRecordFlags *)(recordCursor + IFF_OFFSET(CEnemyRenderRecord, flags)))->enabled;
  item = ((EnemyRenderExtra*)recordCursor)->playerItem;
  value = ((EnemyRenderExtra*)recordCursor)->listValue;
   playerEntry.playerIndexByte = playerIndex; playerEntry.enabled = (bool)enabled; playerEntry.bits.opt0a = 0; playerEntry.index = playerIndex; playerEntry.item = item; playerEntry.value = value;
  strncpy((char *)playerEntry.name1, String_FormatTemporary(playerProfileDefaultNameStrings, playerIndex + 1), 0x16);
		strncpy((char *)playerEntry.name2, String_FormatTemporary(playerProfileDefaultNameStrings, playerIndex + 1), 0x16);
  ListAlloc allocator;
  unsigned int *allocated;
  unsigned int *volatile savedAllocation = allocated = (unsigned int *)new ListRawNode;
  ListNodeGuard guard(allocated, allocator, sentinel);
  ListPayload *dst = (ListPayload *)(guard.node + 2);
   new (dst) ListPayload(playerEntry);
  unsigned int *entry = guard.node;
  // Payload construction has succeeded. Commit the node before splicing it
  // between old sentinel.prev and sentinel; no payload pointer is retained.
  guard.node = 0;
  ((unsigned int *)sentinel[0])[1] = (unsigned int)entry;
  entry[0] = sentinel[0];
  sentinel[0] = (unsigned int)entry;
  entry[1] = (unsigned int)sentinel;
		++head[0];
 }
	}
}

#pragma use_lmw_stmw reset
// Report that numeric draw information is valid.
int CFontNumDraw_FONT_NUM_DRAW_INFO_vfunc_0() {
	return 1;
}

// Initialize the numeric font renderer full-turn angle cache.
void FontNumDraw_InitAngleConstant() {
	s_fontNumDrawTau = kFloat_6_2831855;
}

extern "C" void CSharedDoc_SetCourseRecordUpdatesEnabled(unsigned char value);

extern "C" void CSharedDoc_SetTurnTimeLimitMs(int value);

extern "C" void CSharedDoc_SetRoundHoleOrder(unsigned char value);

extern "C" void CSharedDoc_SetRoundHoleCount(unsigned char value);

extern "C" void CSharedDoc_SetRoundOptionByte(unsigned char value);

extern "C" unsigned int ByteVector_GetSize(void *arg);

void ByteVector_AppendOne(void *, void *);
void *ByteVector_GetDataPointer(void *);
void *ByteVector_GetOffsetPointer(void *, int);
void *ByteVector_InsertAtIterator(void *, void *, void *);
void *ByteVector_GetElementPtr(void *, int);
unsigned int SharedDoc_GetCurrent();
void SharedDoc_SelectCourse(unsigned char);
void dtor_80248D58(void *, int);

struct TempStore {
	int a, b, c;
	// Initialize temporary three-word byte-vector/iterator storage.
	TempStore() { Struct12_Clear(this); }
	// Release temporary storage without deleting this stack object.
	~TempStore() { dtor_80248D58(this, -1); }
};

// Build front/back/shuffled hole and course arrays, randomize hole variants, and apply round options.
void CourseSelection_ApplyRoundSettings(unsigned char *selectionBytes) {
    // Valid domain: caller provides writable RoundSelection and paired vectors;
    // 1..18 holes, one-based 1..18 IDs, course bytes accepted by SelectCourse
    // (no validity check here). Back mode with count!=18 adds 9, so only counts
    // <=9 produce IDs <=18. Rebuild supports order 0/1/3; another order clears
    // mismatched vectors then falls through to unchecked courses[0]. Equal
    // hole-vector count bypasses BOTH vector rebuild/validation; course vector
    // must already match. Changing course/order alone may retain old entries.
    // RNG budget/order (global rand stream): rebuilding shuffle consumes exactly
    // 18 draws, one AFTER each insertion, including the final unused next-index
    // draw. Then every published hole consumes one draw for variant rand()%3,
    // including calls that retain vectors. Front/back rebuild consumes none
    // before this publication. Do not move, remove, seed or combine these draws.
    // Mutations occur before publication; allocation exceptions leave partial
    // vectors and already-cleared record/timer settings, with no rollback.
    // Caller owns selection and its vectors. Phase 1: disable records/timer and
    // rebuild hole/course vectors only when their count differs from holeCount.
    // Equal counts intentionally retain the existing vectors even if mode changed.
    // Phase 2: copy each hole/course pair to shared round state and randomize
    // its variant byte, then publish count/course/option. Hole numbers are
    // one-based. Caller supplies 1..18 paired entries; the final course[0]
    // read has no empty check and shared variant storage holds only 18 bytes.
	CSharedDoc_SetCourseRecordUpdatesEnabled(0);
	CSharedDoc_SetTurnTimeLimitMs(0);
	CSharedDoc_SetRoundHoleOrder(((RoundSelection*)selectionBytes)->holeOrder);
	if (((RoundSelection*)selectionBytes)->holeCount != ByteVector_GetSize((selectionBytes + IFF_OFFSET(RoundSelection, holes)))) {
		ByteVector_Clear((selectionBytes + IFF_OFFSET(RoundSelection, holes)));
		ByteVector_Clear((selectionBytes + IFF_OFFSET(RoundSelection, courses)));
		switch ((signed char)((RoundSelection*)selectionBytes)->holeOrder) {
		case kHoleOrderFront:
			for (int i = 1; i <= ((RoundSelection*)selectionBytes)->holeCount; ++i) {
				unsigned char value = i;
				ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, holes)), &value);
				ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, courses)), selectionBytes);
			}
			break;
		case kHoleOrderBack:
			if (((RoundSelection*)selectionBytes)->holeCount == kCourseHoleCount) {
				for (int i = 1; i <= ((RoundSelection*)selectionBytes)->holeCount; ++i) {
					unsigned char value = i;
					ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, holes)), &value);
					ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, courses)), selectionBytes);
				}
			} else {
				for (int i = 1; i <= ((RoundSelection*)selectionBytes)->holeCount; ++i) {
					unsigned char value = i + kCourseHalfHoleCount;
					ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, holes)), &value);
					ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, courses)), selectionBytes);
				}
			}
			break;
		case kHoleOrderShuffle: {
			TempStore shuffledHoles;
			int insertionIndex = 0;
			for (int i = 1; i <= kCourseHoleCount; ++i) {
				unsigned char value = i;
				void *insertionPosition;
				void *holePosition = ByteVector_GetDataPointer(&shuffledHoles);
				insertionPosition = ByteVector_GetOffsetPointer(&holePosition, insertionIndex);
				ByteVector_InsertAtIterator(&shuffledHoles, &insertionPosition, &value);
				unsigned int shuffledCount = ByteVector_GetSize(&shuffledHoles);
				unsigned int random = rand();
				unsigned int randomQuotient = random / shuffledCount;
				insertionIndex = (int)(random - randomQuotient * shuffledCount);
			}
			for (int i = 0; i < ((RoundSelection*)selectionBytes)->holeCount; ++i) {
				void *holePosition = ByteVector_GetElementPtr(&shuffledHoles, i);
				ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, holes)), holePosition);
				ByteVector_AppendOne((selectionBytes + IFF_OFFSET(RoundSelection, courses)), selectionBytes);
			}
			break;
		}
		default:
			break;
		}
	}
	for (unsigned int i = 0; i < ByteVector_GetSize((selectionBytes + IFF_OFFSET(RoundSelection, holes))); ++i) {
		unsigned char holeNumber = *(unsigned char *)ByteVector_GetElementPtr((selectionBytes + IFF_OFFSET(RoundSelection, holes)), i);
		*(unsigned char *)(i + SharedDoc_GetCurrent() + IFF_OFFSET(SharedRoundArrays, holes)) = holeNumber;
		unsigned char courseIndex = *(unsigned char *)ByteVector_GetElementPtr((selectionBytes + IFF_OFFSET(RoundSelection, courses)), i);
		*(unsigned char *)(i + SharedDoc_GetCurrent() + IFF_OFFSET(SharedRoundArrays, courses)) = courseIndex;
		int random = rand();
		unsigned char variant = (unsigned char)(random % kHoleVariantCount);
		*(unsigned char *)(i + SharedDoc_GetCurrent() + IFF_OFFSET(SharedRoundArrays, randomVariants)) = variant;
	}
	CSharedDoc_SetRoundHoleCount((unsigned char)ByteVector_GetSize((selectionBytes + IFF_OFFSET(RoundSelection, holes))));
	SharedDoc_SelectCourse(*(unsigned char *)ByteVector_GetElementPtr((selectionBytes + IFF_OFFSET(RoundSelection, courses)), 0));
	CSharedDoc_SetRoundOptionByte(((RoundSelection*)selectionBytes)->option);
}

// Clear the byte-vector count while preserving storage and the original volatile access order.
void ByteVector_Clear(void *arg) {
	volatile int *field = (volatile int *)&((ByteArrayStorage*)arg)->size;
	*field = *field - *field;
}

// Initialize a three-word storage record to zero.
void Struct12_Clear(void *arg) {
	int *fields = (int *)arg;
	fields[0] = 0;
	fields[1] = 0;
	fields[2] = 0;
}

// Enable or disable course-record updates for the shared round.
void CSharedDoc_SetCourseRecordUpdatesEnabled(unsigned char value) {
	AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled = value;
}

// Set the shared round turn limit in milliseconds.
void CSharedDoc_SetTurnTimeLimitMs(int value) {
	*((int *)(((char*)&AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs))) = value;
}

// Set the shared round hole-order selector.
void CSharedDoc_SetRoundHoleOrder(unsigned char value) {
	AsCSharedDoc(g_sharedDoc)->roundHoleOrder = value;
}

// Set the number of holes in the shared round.
void CSharedDoc_SetRoundHoleCount(unsigned char value) {
	AsCSharedDoc(g_sharedDoc)->roundHoleCount = value;
}

// Store the round option byte immediately following the shared hole count.
void CSharedDoc_SetRoundOptionByte(unsigned char value) {
	((SharedRoundOption*)g_sharedDoc)->option = value;
}

// Return the byte-vector element count.
unsigned int ByteVector_GetSize(void *arg) {
	return ((PointerArrayStorage*)arg)->size;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622E4", _ctors_u802534D8_0, TextureCoordinateRegistry_InitAngleAndBpGuards);
STATIC_INIT_ENTRY(".CRT$XCU804622E4", _ctors_u802534D8_1, FontNumDraw_InitAngleConstant);
#pragma pop

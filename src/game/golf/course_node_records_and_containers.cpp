#include <decomp/ct_member.h>
#include "MSL/error_message.h"
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
#include "classes/CNodeContainer.h"
#define AsCNodeContainer(p) ((CNodeContainer*)(p))

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char courseModelResourceStrings[];
}
}

// Translation unit 0x801C06BC-0x801C1AC0 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// Address-only views for the small course records initialized in this unit.
struct CourseMatrixRecord { char prefix[0xC]; unsigned int transform[16]; };
struct CourseVectorRecord { char prefix[0x30]; unsigned int values[3]; };
struct RivalShotPositionThinkerView { char prefix[0x40]; void* shotPositionThinker; };
struct CourseBoundsView { char prefix[0x44]; float minimum[3], maximum[3]; };
struct CourseIntVector { unsigned int values[3]; };
// Two nested vectors: the outer vector holds 32-byte polymorphic node values;
// each value owns a small name string and a vector of opaque 12-byte triples.
// Triple words are copied unchanged: neither coordinates nor ownership of any
// pointer-shaped word is established. metadataWord is also opaque, not a count.
struct NodePayloadWords { unsigned int word[3]; };
struct NodeStringWords { unsigned int tagWord, lengthOrInline, dataOrInline; };
struct NodePayloadVector { NodePayloadWords* data; unsigned int size, capacity; };
struct NodeValueStorage {
    unsigned int vtableWord;
    NodeStringWords name;
    unsigned int metadataWord;
    NodePayloadVector entries;
};
struct NodeValueVector { NodeValueStorage* data; unsigned int size, capacity; };
typedef char NodeValueStorage_size[(sizeof(NodeValueStorage) == 0x20) ? 1 : -1];

enum CourseContainerLimits { kMaxTwelveByteEntries = 0x15555555U,
    kMaxFourByteEntries = 0x3FFFFFFFU, kHalfFourByteLimit = 0x2AAAAAAAU,
    kMaxNodeValues = 0x07FFFFFFU,
    kNodeGrowthThreeFifthsEnd = 0x02AAAAAAU,
    kNodeGrowthOneHalfEnd = 0x05555554U };

extern char __vt__14CNodeContainer[];

extern "C" {

void Vec3_Zero(void* object);
void* appMallocDummy__FUl(unsigned int size);
void* __nwa__FUl(unsigned int size);
void String_ReserveCapacity(void* object, void* value);
void String_ReplaceRange(void* object, int, int, unsigned int, unsigned int, void*);
void Matrix_CopyAffineElements(void* object, void* source);
void appFreeUni__FPv(void* object);
void __dl__FPv(void* object);
void* __nw__FUl(unsigned int size);
void __dl__FPv(void* object);
void __dla__FPv(void* object);
void* memcpy(void* destination, const void* source, unsigned int size);
void* memset(void* destination, int value, unsigned int size);
extern char courseModelResourceStrings[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
void CNodeContainer_CopyConstructAt(void* object, void* destination, void* value);
void CNodeRecordVector_GrowAndAppend(void* object, void* value);

// Empty lifecycle hook retained for its original entry point.
void CourseCameraRecord_DefaultConstructNoOp() {}

// Empty lifecycle hook retained for its original entry point.
void CourseSoundEmitter_DefaultConstructNoOp() {}

#pragma use_lmw_stmw on
// Copy a complete 4x4 transform into the record without reordering source reads.
void TransformRecord_CopyMatrix4x4(void* object, const unsigned int* source) {
	unsigned int w7 = source[7];
	unsigned int w0 = source[0];
	unsigned int w1 = source[1];
	unsigned int w2 = source[2];
	unsigned int w3 = source[3];
	unsigned int w4 = source[4];
	unsigned int w5 = source[5];
	unsigned int w8 = source[8];
	unsigned int w9 = source[9];
	unsigned int w6 = source[6];
	unsigned int w10 = source[10];
	unsigned int w11 = source[11];
	unsigned int w12 = source[12];
	unsigned int w13 = source[13];
	unsigned int w14 = source[14];
	unsigned int w15 = source[15];
	unsigned int* d = ((CourseMatrixRecord*)object)->transform;
	d[0] = w0;
	d[1] = w1;
	d[2] = w2;
	d[3] = w3;
	d[4] = w4;
	d[5] = w5;
	d[6] = w6;
	unsigned int savedW = w8;
 d[7] = w7;
 d[8] = savedW;
	d[9] = w9;
	d[10] = w10;
	d[11] = w11;
	d[12] = w12;
	d[13] = w13;
	d[14] = w14;
	d[15] = w15;
}

#pragma opt_propagation off
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control Control_801C0A28;
struct BufferGuard_801C0A28 {
	char* data;
	char* flag;
	// Release message bytes if control-block construction throws before adoption.
	~BufferGuard_801C0A28() { if (data) __dla__FPv(data); }
};
struct ErrorBase_801C0A28 {
	void* vtable;
	char* text;
	void* control;
	// Preserve the original empty base-destructor boundary for exception construction.
	~ErrorBase_801C0A28() { }
};
namespace std {
struct length_error : ErrorBase_801C0A28 {
	// Copy an owned shared message through the register-temporary constructor variant.
	length_error(const char* str, int) {
		register const char* message = str;
		*(void**)this = __vt__Q23std11logic_error;
		struct Flag { char value; char pad[3]; };
		Flag flag; flag.value = 0;
		unsigned long n = strlen(message) + 1;
		char* allocated = (char*)__nwa__FUl(2 * n - n);
		text = allocated;
		{
			BufferGuard_801C0A28 guard;
			guard.data = allocated;
			guard.flag = &flag.value;
			control = new Control_801C0A28(allocated);
			guard.data = 0;
		}
		strcpy(text, message);
		*(void**)this = __vt__Q23std12length_error;
	}
	// Copy an owned shared message through the ordinary constructor variant.
	length_error(const char* str) {
		*(void**)this = __vt__Q23std11logic_error;
		struct Flag { char value; char pad[3]; };
		Flag flag; flag.value = 0;
		unsigned long n = strlen(str) + 1;
		char* allocated = (char*)__nwa__FUl(2 * n - n);
		text = allocated;
		{
			BufferGuard_801C0A28 guard;
			guard.data = allocated;
			guard.flag = &flag.value;
			control = new Control_801C0A28(allocated);
			guard.data = 0;
		}
		strcpy(text, str);
		*(void**)this = __vt__Q23std12length_error;
	}
	~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
// Construct length_error in aligned unwind storage and transfer it to the throw runtime.
inline void capacityError_801C0A28(const char* message) { {
    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
    std::length_error* error = (std::length_error*)storage.bytes;
    error->__ct(message);
    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + 0x14, error,
        (void*)exception_runtime::__dt__Q23std12length_errorFv);
} }
#pragma use_lmw_stmw on

// Copy a node name, metadata and entry vector into uninitialized storage, preserving exception cleanup.
// destination may be null (no-op); otherwise it must be uninitialized and must
// not overlap value. object is an allocator-context receiver, unused here.
// Node container layout: +0 vtable, +4..0xF small string, +0x10 metadata,
// +0x14 owned 12-byte-entry vector (data/size/capacity). Entries are copied as
// raw three-word payloads; their component meanings are unresolved here.
// Small string: bit 31 of +4 selects heap form (+8 length, +0xC pointer);
// otherwise all three words contain the inline representation. NodeValueStorage gives both inline and growth paths the same field map.
void CNodeContainer_CopyConstructAt(void* object, void* destination, void* value) {
	NodeValueStorage* dst = (NodeValueStorage*)destination;
	NodeValueStorage* src = (NodeValueStorage*)value;
	if (destination == 0)
		return;
	try {
	// Phase 1: install node vtable, then inline-copy or deep-copy the name string.
	dst->vtableWord = (unsigned int)((char*)&__vt__14CNodeContainer);
	if ((src->name.tagWord >> 31) == 0) {
		dst->name.tagWord = src->name.tagWord;
		dst->name.lengthOrInline = src->name.lengthOrInline;
		dst->name.dataOrInline = src->name.dataOrInline;
	} else {
		dst->name.tagWord = 0;
		dst->name.lengthOrInline = 0;
		dst->name.dataOrInline = 0;
		String_ReserveCapacity(&dst->name.tagWord, (void*)src->name.lengthOrInline);
		String_ReplaceRange(&dst->name.tagWord, 0, 0, src->name.dataOrInline, src->name.lengthOrInline + src->name.dataOrInline,
		            (char*)&dst->name.tagWord + 8);
	}
	// Phase 2: initialize metadata and an empty owned entry-vector descriptor.
	NodePayloadVector* out = &dst->entries; // 0x801C0A28: vector at +0x14
	out->data = 0;
	dst->metadataWord = src->metadataWord; // +0x10
	out->size = 0;
	out->capacity = 0;
	NodePayloadWords* begin = src->entries.data;
	NodePayloadWords* end = begin + src->entries.size;
	unsigned int count = end - begin;
	// Phase 3: allocate exactly the source count and advance live size per copied entry.
	if (count != 0) {
		if (count > kMaxTwelveByteEntries) {
			capacityError_801C0A28((const char*)courseModelResourceStrings);
		}
		out->data = (NodePayloadWords*)appMallocDummy__FUl(count * 12);
		out->capacity = count;
		NodePayloadWords* in = begin;
		NodePayloadWords* dstEntry = (NodePayloadWords*)out->data + out->size;
		while (in != end) {
			if (dstEntry != 0) {
				*dstEntry = *in;
			}
			++in;
			++dstEntry;
			out->size++;
		}
	}
	} catch (...) { throw; }
}

// Allocate a list node, copy its name, position and affine transform, and insert it after the sentinel.
void CourseNodeList_PrependRecord(void* object, unsigned char* value) {
	// List owns a circular sentinel at +4 and count at +0. Each 0x64-byte
	// node owns its allocation: +0/+4 links, +8 32 opaque record/name bytes,
	// +0x28 Vec3 position, +0x34 affine matrix. value borrows the payload only.
	struct Link { Link* next; Link* prev; };
	struct Node {
		Link link;
		unsigned char data[0x20];
		float x, y, z;
		char affineTransform[0x30];
	};
	struct List { unsigned int count; Link sentinel; };
	struct NodeGuard {
		Link* sentinel;
		Node* node;
		// Own the unlinked allocation until its payload has been initialized.
		NodeGuard(Link* s, Node* n) : sentinel(s), node(n) {}
		// Transfer the initialized node to the list-splice phase.
		Node* release() { Node* n = node; node = 0; return n; }
		// Free a node whose construction failed before linking.
		~NodeGuard() { if (node) appFreeUni__FPv(node); }
	};
	List* list = (List*)object;
	Link* sentinel = &list->sentinel;
	Node* node = (Node*)appMallocDummy__FUl(0x64);
	NodeGuard guard(sentinel, node);
	try {
	node->data[0] = value[0]; node->data[1] = value[1];
	node->data[2] = value[2]; node->data[3] = value[3];
	node->data[4] = value[4]; node->data[5] = value[5];
	node->data[6] = value[6]; node->data[7] = value[7];
	node->data[8] = value[8]; node->data[9] = value[9];
	node->data[10] = value[10]; node->data[11] = value[11];
	node->data[12] = value[12]; node->data[13] = value[13];
	node->data[14] = value[14]; node->data[15] = value[15];
	node->data[16] = value[16]; node->data[17] = value[17];
	node->data[18] = value[18]; node->data[19] = value[19];
	node->data[20] = value[20]; node->data[21] = value[21];
	node->data[22] = value[22]; node->data[23] = value[23];
	node->data[24] = value[24]; node->data[25] = value[25];
	node->data[26] = value[26]; node->data[27] = value[27];
	node->data[28] = value[28]; node->data[29] = value[29];
	node->data[30] = value[30]; node->data[31] = value[31];
	node->x = *(float*)(value + 0x20);
	node->y = *(float*)(value + 0x24);
	node->z = *(float*)(value + 0x28);
	Matrix_CopyAffineElements(node->affineTransform, value + 0x2c);
	} catch (...) { throw; }
	// Commit: splice only after all payload copies complete; count changes last.
	Node* inserted = guard.release();
	Link* first = sentinel->next;
	first->prev = &inserted->link;
	inserted->link.next = first;
	sentinel->next = &inserted->link;
	inserted->link.prev = sentinel;
	list->count++;
}

// Clear the three stored vector words in the course record.
void CourseRecord_ClearVectorWords(void* object) {
	((CourseVectorRecord*)object)->values[0] = 0;
	((CourseVectorRecord*)object)->values[1] = 0;
	((CourseVectorRecord*)object)->values[2] = 0;
}

// Initialize an empty three-word vector descriptor.
void CourseIntVector_InitEmpty(void* object) {
	((CourseIntVector*)object)->values[0] = 0;
	((CourseIntVector*)object)->values[1] = 0;
	((CourseIntVector*)object)->values[2] = 0;
}

// Test whether the two stored identifiers differ.
int CourseModelPointerIterator_NotEqual(unsigned int* a, unsigned int* b) {
	return *a != *b;
}

// Return the rival manager's stored shot-position thinker.
void* CRival_GetShotPositionThinker(void* object) {
	return ((RivalShotPositionThinkerView*)object)->shotPositionThinker;
}

// Zero the two bounding vectors and return the record.
void* CourseRecord_ZeroBoundingVectors(void* object) {
	Vec3_Zero(((CourseBoundsView*)object)->minimum);
	Vec3_Zero(((CourseBoundsView*)object)->maximum);
	return object;
}

// Append a node in spare capacity or delegate to the checked vector growth path.
void CNodeContainer_Append(void* object, void* value) {
	NodeValueVector* vector = (NodeValueVector*)object;
	if (vector->size < vector->capacity) {
		CNodeContainer_CopyConstructAt((char*)object + 8,
		            vector->data + vector->size, value);
		vector->size = vector->size + 1;
	} else {
		CNodeRecordVector_GrowAndAppend(object, value);
	}
}

extern char __vt__14CNodeContainer[];
#pragma opt_propagation off
struct Flag_0C7C { volatile char value; char pad[3]; };
// Placement construction returns caller-owned storage without allocating.
void* operator new(size_t, void* p) { return p; }
// Throw the node-vector length error through the allocator-tag constructor variant.
inline void error_0C7C(const char* str) {
	Flag_0C7C flag; flag.value = 0;
	{
	    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
	    std::length_error* error = (std::length_error*)storage.bytes;
	    error->__ct(str, 0);
	    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + 0x14, error,
	        (void*)exception_runtime::__dt__Q23std12length_errorFv);
	}
}
// Compiler-facing constructors below express the same NodeValueStorage model.
// Distinct string/range constructors preserve different preload and unwind
// boundaries: merging them changes code even though their logical values agree.
// ABI tag bytes have no node payload meaning and must not be repurposed.
// Private type map for the growth path (suffix identifies this reconstruction,
// not a field's meaning): Entry = opaque 12-byte node payload; Entries = owned
// payload vector; String = small string; Record = 32-byte CNodeContainer value;
// Vector = outer vector of Records; Growth = temporary owner during append.
// a/b/c are preserved raw words, not established x/y/z or pointer relationships.
struct Entry_0C7C {
	unsigned int a, b, c;
	// Copy all payload words with the original per-entry rethrow boundary.
	Entry_0C7C(const Entry_0C7C& x) {
		try {
			unsigned int word0 = x.a;
			unsigned int word1 = x.b;
			a = word0;
			b = word1;
			c = x.c;
		} catch (...) { throw; }
	}
};
struct EntriesBase_0C7C {
	Entry_0C7C* data;
	unsigned int size, capacity;
	// Initialize an empty owned payload vector.
	EntriesBase_0C7C() : data(0), size(0), capacity(0) {}
	// Store enclosing metadata and initialize the empty payload vector together.
	EntriesBase_0C7C(unsigned int* field, unsigned int value) : data((*field = value, (Entry_0C7C*)0)), size(0), capacity(0) {}
	// Release payload storage; entries themselves hold only raw words here.
	~EntriesBase_0C7C() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
struct Entries_0C7C : EntriesBase_0C7C {
	// Deep-copy [first,last) while initializing the enclosing record metadata.
	Entries_0C7C(Entry_0C7C* last, Entry_0C7C* first, unsigned int* field, unsigned int value) : EntriesBase_0C7C(field, value) {
		Entry_0C7C* out;
		unsigned int count;
		count = last - first;
		if (count) {
			Flag_0C7C alloc; alloc.value = 0;
			if (count > kMaxTwelveByteEntries) error_0C7C(courseModelResourceStrings);
			data = (Entry_0C7C*)appMallocDummy__FUl(count * 12);
			capacity = count;
			Flag_0C7C alloc2; alloc2.value = 0;
			out = data + size;
			while (first != last) {
				new (out) Entry_0C7C(*first);
				++first; ++out; ++size;
			}
		}
	}
	// Deep-copy an existing owned payload vector.
	Entries_0C7C(const Entries_0C7C& x) {
		Entry_0C7C* out;
		unsigned int count;
		Entry_0C7C* last;
		Entry_0C7C* first;
		first = x.data;
		last = first + x.size;
		count = last - first;
		if (count) {
			Flag_0C7C alloc; alloc.value = 0;
			if (count > kMaxTwelveByteEntries) error_0C7C(courseModelResourceStrings);
			data = (Entry_0C7C*)appMallocDummy__FUl(count * 12);
			capacity = count;
			{
				Flag_0C7C alloc; alloc.value = 0;
				out = data + size;
				while (first != last) {
					new (out) Entry_0C7C(*first);
					++first; ++out; ++size;
				}
			}
		}
	}
	~Entries_0C7C();
};
struct String_0C7C {
	// The final two words also hold inline bytes when the heap flag is clear.
	unsigned int flag, lengthWord, heapDataWord;
	// Copy inline bytes or allocate/copy heap bytes using a preloaded tag word.
	String_0C7C(const String_0C7C& x, unsigned int f) {
		if ((f >> 31) == 0) {
			flag = f;
			unsigned int d = x.lengthWord; unsigned int s = x.heapDataWord;
			lengthWord = d; heapDataWord = s;
		} else {
			unsigned int d = x.lengthWord;
			flag = 0; lengthWord = 0; heapDataWord = 0;
			String_ReserveCapacity(this, (void*)d);
			Flag_0C7C allocator;
			char allocatorValue = allocator.value;
			unsigned int s = x.heapDataWord;
			unsigned int d2 = x.lengthWord;
			Flag_0C7C copy; copy.value = allocatorValue;
			String_ReplaceRange(this, 0, 0, s, s + d2, &copy);
		}
	}
	// Deep-copy the small string from its own tag word.
	String_0C7C(const String_0C7C& x) {
		if ((x.flag >> 31) == 0) {
			unsigned int inlineTag = x.flag;
			unsigned int inlineMiddle = x.lengthWord;
			flag = inlineTag;
			lengthWord = inlineMiddle;
			heapDataWord = x.heapDataWord;
		}
		else {
			flag = 0; lengthWord = 0; heapDataWord = 0;
			String_ReserveCapacity(this, (void*)x.lengthWord);
			Flag_0C7C allocator;
			Flag_0C7C copy; copy.value = allocator.value;
			String_ReplaceRange(this, 0, 0, x.heapDataWord, x.heapDataWord + x.lengthWord, &copy);
		}
	}
	// Release only the heap representation; inline characters reside in the object.
	~String_0C7C() { if (flag >> 31) appFreeUni__FPv((void*)heapDataWord); }
};
struct Record_0C7C {
	void* vtable;
	String_0C7C string;
	unsigned int metadataWord;
	Entries_0C7C entries;
	// Deep-copy a node value, retaining the constructor's member-unwind boundary.
	Record_0C7C(const Record_0C7C& x) try : vtable(__vt__14CNodeContainer), string(x.string), metadataWord(x.metadataWord), entries(x.entries) {} catch (...) {}
	// Construct the appended node using its preloaded string tag and metadata-aware range copy.
	Record_0C7C(unsigned int f, const Record_0C7C& x) try : vtable(__vt__14CNodeContainer), string(x.string, f), entries(x.entries.data + x.entries.size, x.entries.data, &metadataWord, x.metadataWord) {} catch (...) {}
};
// Semantic aliases preserve compiler-facing type identities in exception metadata.
typedef Entry_0C7C OpaqueNodeTriple;
typedef Entries_0C7C OwnedNodePayloads;
typedef String_0C7C OwnedNodeName;
typedef Record_0C7C OwnedNodeValue;
struct Vector_0C7C { Record_0C7C* data; unsigned int size, capacity; };
typedef Vector_0C7C OwnedNodeVector;
struct Virtual_0C7C { 
virtual ~Virtual_0C7C();
};
// Destroy one node value virtually, leaving its outer vector allocation intact.
inline void destroy_0C7C(Record_0C7C* p) { ((Virtual_0C7C*)p)->~Virtual_0C7C(); }
struct Growth_0C7C {
	OwnedNodeValue* data;
	unsigned int size, capacity;
	unsigned int* capacityField;
	unsigned int offset;
	// Begin an empty replacement owner; the live constructed range starts at offset.
	Growth_0C7C(unsigned int* p) : data(0), size(0), capacity(0), capacityField(p), offset(0) {}
	// Destroy held live records backwards, then release the allocation.
	~Growth_0C7C() {
		if (data) {
			OwnedNodeValue* end;
			unsigned int count;
			count = size;
			end = data + count;
			Flag_0C7C alloc; alloc.value = 0;
			size -= count;
			while (count) { destroy_0C7C(--end); --count; }
			appFreeUni__FPv(data);
		}
	}
};
// Choose the larger growth amount by reference, preserving allocator expression lifetimes.
inline const unsigned int& max_0C7C(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
// Construct the new tail first, so an appended value may refer into the old vector.
inline void insert_0C7C(Growth_0C7C& guard, OwnedNodeValue* dst, OwnedNodeValue* value) {
	unsigned int count = 1;
	Flag_0C7C alloc4; alloc4.value = 0;
	while (count) {
		new (dst) Record_0C7C(value->string.flag, *value);
		--count; ++dst; ++guard.size;
	}
}
// Prepend old records backwards; offset shrinks and live size grows after each construction.
inline void copyback_0C7C(Growth_0C7C& guard, OwnedNodeValue* first, OwnedNodeValue* last) {
	OwnedNodeValue* dst = guard.data + guard.offset;
	Flag_0C7C alloc5; alloc5.value = 0;
	while (last > first) {
		--dst; --last;
		new (dst) Record_0C7C(*last);
		--guard.offset; ++guard.size;
	}
}
// Commit the new descriptor, destroy old records, and leave old allocation with the guard.
inline void replace_0C7C(OwnedNodeVector* vector, Growth_0C7C& guard) {
	OwnedNodeValue* begin;
	OwnedNodeValue* end;
	OwnedNodeValue* oldData = vector->data;
	begin = oldData + guard.offset;
	OwnedNodeValue* replacement = guard.data;
	end = begin + vector->size;
	unsigned int newSize = guard.size;
	unsigned int oldSize = vector->size;
	unsigned int oldCapacity = vector->capacity;
	guard.data = oldData;
	Flag_0C7C alloc6; alloc6.value = 0;
	vector->capacity = guard.capacity;
	guard.capacity = oldCapacity;
	vector->data = replacement;
	vector->size = newSize;
	guard.size = oldSize;
	while (end > begin) destroy_0C7C(--end);
	guard.size = 0;
}
// Grow node storage, copy-construct the appended node and existing nodes, then swap ownership safely.
// Precondition: a valid outer vector at its growth boundary (size == capacity).
// Counts/capacities are RECORDS, 32 bytes each; 0x07FFFFFF is the byte-overflow
// ceiling. The requested value is borrowed and may be a live element of this
// vector. No old record is destroyed until the replacement range is complete.
void CNodeRecordVector_GrowAndAppend(void* object, void* value) {
	OwnedNodeVector* vector;
	unsigned int capacity;
	unsigned int oldLimit;
	OwnedNodeValue* last;
	OwnedNodeValue* secondDst;
	OwnedNodeValue* dst;
	unsigned int count;
	OwnedNodeValue* first;
	OwnedNodeValue* begin;
	OwnedNodeValue* end;
	// Phase 1: check growth arithmetic before allocating; allocator tags retain
	// constructor/unwind boundaries and are not additional stored node fields.
	vector = (OwnedNodeVector*)object;
	Flag_0C7C alloc; alloc.value = 0;
	Flag_0C7C alloc2; alloc2.value = 0;
	if (kMaxNodeValues - vector->capacity < 1U) {
		Flag_0C7C flag; flag.value = 0;
		{
		    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
		    std::length_error* error = (std::length_error*)storage.bytes;
		    error->__ct(courseModelResourceStrings, 0);
		    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + 0x14, error,
		        (void*)exception_runtime::__dt__Q23std12length_errorFv);
		}
	}
	unsigned int currentSize = vector->size;
	unsigned int currentCapacity = vector->capacity;
	unsigned int* capacityField = &vector->capacity;
    // At this boundary delta == 1; retain the general expression and its overflow
    // tests. Capacity grows by max(required, floor(3*(old+1)/5)), then by half
    // above the first threshold, and finally saturates at the 32-byte limit.
	unsigned int delta = currentSize + 1 - currentCapacity;
	Growth_0C7C guard(capacityField);
	oldLimit = vector->capacity;
	unsigned int secondGrowth;
	unsigned int firstGrowth;
	if (delta > 0x7FFFFFFU - oldLimit) error_0C7C(courseModelResourceStrings);
	if (oldLimit < kNodeGrowthThreeFifthsEnd) {
		firstGrowth = ((oldLimit + 1) * 3) / 5;
		capacity = oldLimit + max_0C7C(firstGrowth, delta);
	} else if (oldLimit < kNodeGrowthOneHalfEnd) {
		secondGrowth = (oldLimit + 1) / 2;
		capacity = oldLimit + max_0C7C(secondGrowth, delta);
	} else capacity = kMaxNodeValues;
	Flag_0C7C alloc3; alloc3.value = 0;
	if (capacity > 0x7FFFFFFU) error_0C7C(courseModelResourceStrings);
	// Phase 2: guard owns new storage, append the requested value before copying
	// old elements. Phase 3 copies backwards to make [offset,offset+size) complete.
	OwnedNodeValue* data = (OwnedNodeValue*)appMallocDummy__FUl(capacity * 32);
	guard.data = data;
	guard.capacity = capacity;
	guard.offset = vector->size;
	dst = data + guard.offset + guard.size;
	insert_0C7C(guard, dst, (OwnedNodeValue*)value);
	first = vector->data;
	last = first + vector->size;
	copyback_0C7C(guard, first, last);
	// Invariant: offset==0 and size==oldSize+1; every replacement record now
	// owns its own name/payload allocation, so destroying the old values is safe.
	// Phase 4: swap descriptor ownership; destroy old values, then guard frees old storage.
	replace_0C7C(vector, guard);
}

#pragma opt_propagation off
// Release integer-vector storage, clear its count and optionally delete the descriptor.
void* dtor_801C15D8(MSLPointerVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

#pragma opt_propagation off
struct GrowthGuard_801C1658 {
	int* data;
	unsigned int size;
	unsigned int capacity;
	unsigned int* capacityField;
	unsigned int offset;
	// Track an integer-vector replacement independently of the live descriptor.
	GrowthGuard_801C1658(void* vector, unsigned int* alloc)
		: data(0), size(0), capacity(0), capacityField(alloc), offset(0) {}
	// Release the allocation still owned by the temporary after commit or unwind.
	~GrowthGuard_801C1658() {
		if (data) {
			size = 0;
			appFreeUni__FPv(data);
		}
	}
};

// Return the larger integer-vector growth candidate by reference.
inline const unsigned int& max_801C1658(const unsigned int& a, const unsigned int& b) {
	return a < b ? b : a;
}

// Construct and throw the integer-vector length error without changing unwind storage.
inline void error_1658(const char* str) { register const char* message = str; {
    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
    std::length_error* error = (std::length_error*)storage.bytes;
    error->__ct(message, 0);
    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + 0x14, error,
        (void*)exception_runtime::__dt__Q23std12length_errorFv);
} }
// Append one integer, growing and transferring the vector storage when capacity is exhausted.
void CourseIntVector_Append(void* object, const int* value) {
	// Phase 1: use spare capacity, otherwise validate count-based growth (4 bytes/int).
	struct Vector { int* data; unsigned int size; unsigned int capacity; };
	int count;
	int* begin;
	unsigned int byteCount;
	Vector* vector = (Vector*)object;
	unsigned int limit;
	unsigned int size = vector->size;
	limit = vector->capacity;
	if (size < limit) {
		unsigned int next = size + 1;
		int element = *value;
		vector->size = next;
		vector->data[next - 1] = element;
		return;
	}
	if (kMaxFourByteEntries - limit < 1U) {
		error_1658(courseModelResourceStrings);
	}
	unsigned int delta = vector->size + 1 - vector->capacity;
	unsigned int* capacityField = &vector->capacity;
	GrowthGuard_801C1658 guard(vector, capacityField);
	unsigned int capacity = vector->capacity;
	unsigned int secondGrowth;
	unsigned int firstGrowth;
	if (delta > kMaxFourByteEntries - capacity) {
		{
		    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
		    std::length_error* error = (std::length_error*)storage.bytes;
		    error->__ct(courseModelResourceStrings, 0);
		    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + 0x14, error,
		        (void*)exception_runtime::__dt__Q23std12length_errorFv);
		}
	}
	if (capacity < kMaxTwelveByteEntries) {
		firstGrowth = ((capacity + 1) * 3) / 5;
		capacity += max_801C1658(firstGrowth, delta);
	} else if (capacity < kHalfFourByteLimit) {
		secondGrowth = (capacity + 1) / 2;
		capacity += max_801C1658(secondGrowth, delta);
	} else {
		capacity = 0x3FFFFFFF;
	}
	if (capacity > kMaxFourByteEntries) {
		{
		    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
		    std::length_error* error = (std::length_error*)storage.bytes;
		    error->__ct(courseModelResourceStrings, 0);
		    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + 0x14, error,
		        (void*)exception_runtime::__dt__Q23std12length_errorFv);
		}
	}
	// Phase 2: copy the borrowed new value before copying/zeroing the old allocation,
	// allowing value to point at an existing vector element.
	int* newData = (int*)appMallocDummy__FUl(capacity * 4);
	guard.offset = vector->size;
	(newData + guard.offset)[guard.size] = *value;
	guard.data = newData;
	guard.capacity = capacity;
	++guard.size;
	begin = vector->data;
	int* end = begin + vector->size;
	count = ((char*)end - (char*)begin) / 4;
	guard.offset -= count;
	byteCount = count * 4;
	memcpy(newData + guard.offset, begin, byteCount);
	memset(begin, 0, byteCount);
	guard.size += count;
	// Phase 3: transfer descriptor ownership; the guard then frees the old allocation.
	int* oldData = vector->data;
	unsigned int newSize = guard.size;
	int* replacement = guard.data;
	vector->size = 0;
	guard.data = oldData;
	unsigned int oldCapacity = vector->capacity;
	vector->capacity = guard.capacity;
	guard.capacity = oldCapacity;
	vector->data = replacement;
	vector->size = newSize;
	guard.size = 0;
}

} // extern "C"

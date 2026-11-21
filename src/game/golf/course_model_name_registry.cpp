#include <decomp/ct_member.h>
#include "MSL/error_message.h"
#include "MSL/vector.h"
// Vector destroyed by dtor_801C2A88; element type not known yet (Elem_801C2A88 is a tag);
// its size reset loads the size twice (form 2).
struct Elem_801C2A88 { unsigned int words[1]; };
namespace std { template <> struct __vec_destroys_elements<Elem_801C2A88> { enum { value = 2 }; static void destroy(Elem_801C2A88*) {} }; }
typedef std::vector<Elem_801C2A88, app::app_allocator<Elem_801C2A88> > Elem_801C2A88Vector;
#include "classes/NameRegistry801C.h"
// Borrow the registry layout; an inline helper changes .text and unwind relocations.
#define AsNameRegistry801C(p) ((NameRegistry801C*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char courseModelResourceStrings[];
}
}

#include "game/string.h"
#include "classes/IntStringTreeNode8013.h"

// Registry entries form a circular live/free list and a separate hash chain.
// A block's first entry is allocation bookkeeping; remaining entries enter the free ring.
// A live entry owns name only when ownsName != 0. Otherwise the caller must keep it alive.
struct RegistryEntry {
    int key;
    const char* name;
    unsigned char ownsName;
    char reserved[3];
    RegistryEntry* previous;
    RegistryEntry* next;
    RegistryEntry* hashNext;
};
struct RegistryTreeNode {
    RegistryTreeNode* left;
    RegistryTreeNode* right;
    unsigned int parentAndColor;
    int key;
    unsigned int value;
};
struct RegistryTree {
    unsigned int count;
    RegistryTreeNode* root;
    RegistryTreeNode* reserved;
    RegistryTreeNode* first;
};
enum {
    kRegistryLockOffset = (int)&((NameRegistry801C*)0)->lock,
    kRegistryPreviousOffset = (int)&((RegistryEntry*)0)->previous,
    kRegistryNextOffset = (int)&((RegistryEntry*)0)->next,
    kRegistryHashNextOffset = (int)&((RegistryEntry*)0)->hashNext,
    kRegistryFirstCharMask = 0x2A,
    kRegistryLastCharMask = 0x15,
    kRegistryLengthShift = 6,
    kLengthErrorTypeOffset = 0x14
};
// Capacities are element counts; the thresholds avoid overflow during growth.
static const unsigned int kPointerVectorMax = 0x3FFFFFFFu;
static const unsigned int kPointerVectorSmallGrowthLimit = 0x15555555u;
static const unsigned int kPointerVectorMediumGrowthLimit = 0x2AAAAAAAu;
static const unsigned int kByteVectorMax = 0xFFFFFFFFu;
static const unsigned int kByteVectorSmallGrowthLimit = 0x55555555u;
static const unsigned int kByteVectorMediumGrowthLimit = 0xAAAAAAAAu;
static const unsigned int kRecordVectorMax = 0x1E1E1E1;
static const unsigned int kRecordVectorSmallGrowthLimit = 0xA0A0A0;
static const unsigned int kRecordVectorMediumGrowthLimit = 0x1414140;

// Translation unit 0x801C1AC0-0x801C3100 (29 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void CourseModelPointerVector_Append(void*, void**);

extern int g_shadowManager;
extern int s_nameRegistryMapDefaultValue;
extern void String_ReplaceRange(void*, void*, int, void*, void*, unsigned char*);
extern void appFreeUni__FPv(void*);
extern void* appMallocUni__FUl(unsigned int);
extern char* strcpy(char*, const char*);
extern void __dl__FPv(void*);
extern void* FxSprayTree_InsertNode(void*, int*, unsigned char, unsigned char, void*);
extern int strcmp(const char*, const char*);
extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern unsigned int strlen(const char*);
extern void* memset(void*, int, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* CourseMaterialMap_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
extern void* IntStringTree_FindInsertPosition(void*, void*, void**, unsigned char*, unsigned char*);
extern void* IntStringTree_InsertAtPosition(void*, void*, unsigned char, unsigned char, void*);
extern void String_ReserveCapacity(int*, int*, int*, int*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void* HeapString_dtor(char*, int);

extern void* dtor_801C709C(void*, int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __throw(void*, void*, void*);
extern void __dt__Q23std12length_errorFv();
extern void* __dt__Q33std6detail16char_array_guardFv(void*, int);
extern "C" void* __dt__Q23std9exceptionFv(void*, int);
extern char courseModelResourceStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control StrRep801C1AC0;

struct Guard801C1AC0 {
	void* p;
	void* q;
	// Free message storage until its ownership transfers to the exception.
	~Guard801C1AC0() { if (p) __dla__FPv(p); }
};

// Keep this throw expansion in the caller: an inline helper changes code and unwind data.
#define THROW_LEN_801C1AC0() do { \
	char* __msg = courseModelResourceStrings; \
	char __marker = 0; \
	void* __exc[3]; \
	void** __excp = __exc; \
	volatile Guard801C1AC0 __guard; \
	__exc[0] = (void*)__vt__Q23std11logic_error; \
	unsigned int __n = strlen(__msg) + 1; \
	char* __buf = (char*)__nwa__FUl(__n * 2 - __n); \
	__exc[1] = __buf; \
	__guard.p = __buf; \
	__guard.q = &__marker; \
	StrRep801C1AC0* __rep = new StrRep801C1AC0(__buf); \
	__exc[2] = __rep; \
	__guard.p = 0; \
	strcpy((char*)__exc[1], __msg); \
	__exc[0] = (void*)__vt__Q23std12length_error; \
	__throw(courseModelResourceStrings + kLengthErrorTypeOffset, __excp, (void*)&__dt__Q23std12length_errorFv); \
} while (0)

// RAII guard: locks the registry mutex on construction, unlocks on destruction.
struct RegistryLockGuard {
	void* object;
	// Acquire one recursive mutex level for registry access.
	RegistryLockGuard(void* ptr) : object(ptr) { OSLockMutex(ptr); }
	// Release this guard's mutex level on normal exit or unwinding.
	~RegistryLockGuard() { OSUnlockMutex(object); }
};

// Release the temporary integer/string payload after insertion copies it.
struct TempKey { int key; int a; int b; int c; ~TempKey(){ HeapString_dtor((char*)this, -1); } };
// Release the temporary narrow string after use.
struct TempObj28 { int x; int y; int z; ~TempObj28(){ ((NString*)(this))->~NString(); } };

// Return the address of the mapped string, inserting an empty string if the key is absent.
// The tree owns the result; insertion copies its temporary key/string payload.
void* IntStringTree_GetOrInsertValue(void* tree, int* integerKey) {
	void* hint;
	unsigned char insertLeft, beforeFirst;
	void* insertionParent = IntStringTree_FindInsertPosition(tree, integerKey, &hint, &insertLeft, &beforeFirst);
	void* result;
	if (hint == 0 || ((RegistryTreeNode*)hint)->key < *integerKey) {
		TempKey key;
		key.key = *integerKey;
		key.a = key.b = key.c = 0;
		TempObj28 emptyString;
		emptyString.x = emptyString.y = emptyString.z = 0;
		void* insertedNode = IntStringTree_InsertAtPosition(tree, insertionParent, insertLeft, beforeFirst, &key);
		// TempKey and TempObj28 each release their owned heap buffer once on scope exit;
		// insertion copies their values and does not transfer ownership of those buffers.
		result = &((RegistryTreeNode*)insertedNode)->key;
	} else result = &((RegistryTreeNode*)hint)->key;
	return (char*)result + 4;
}

// Allocate a registry entry, optionally copy its name, and link it into live and hash lists.
// key is copied by value; copyName requires a nonnull NUL-terminated name. The nested
// locks use the platform's recursive mutex. This routine does not replace duplicate names.
void CourseNameRegistry_Insert(void* registry, int* key, const char* name, unsigned char copyName) {
	char* base = (char*)registry;
	RegistryLockGuard lock0(base + kRegistryLockOffset);
	RegistryLockGuard lock1(base + kRegistryLockOffset);
	void* node = AsNameRegistry801C(base)->freeNodes;
	// Phase 1: replenish the free ring if needed, retaining the allocation in blocks.
	if (node == 0) {
		char* storage = (char*)appMallocUni__FUl(AsNameRegistry801C(base)->nodesPerBlock * sizeof(RegistryEntry));
		void* head;
		{
		void* anchor = base + kRegistryLockOffset;
		head = AsNameRegistry801C(base)->blocks;
		OSLockMutex(anchor);
		if (head == 0) {
			*(void**)&((RegistryEntry*)storage)->previous = storage;
			*(void**)&((RegistryEntry*)storage)->next = storage;
			OSUnlockMutex(anchor);
			head = storage;
		} else {
			*(void**)&((RegistryEntry*)storage)->next = head;
			*(void**)&((RegistryEntry*)storage)->previous = *(void**)&((RegistryEntry*)head)->previous;
			*(void**)&((RegistryEntry*)head)->previous = storage;
			*(void**)(*(char**)(storage + kRegistryPreviousOffset) + kRegistryNextOffset) = storage;
			OSUnlockMutex(anchor);
		}
		}
		AsNameRegistry801C(base)->blocks = head;
		storage += sizeof(RegistryEntry);
		for (int i = 1; i < AsNameRegistry801C(base)->nodesPerBlock; ++i, storage += sizeof(RegistryEntry)) {
			void* anchor = base + kRegistryLockOffset;
			void* next = AsNameRegistry801C(base)->freeNodes;
			OSLockMutex(anchor);
			if (next == 0) {
				*(void**)&((RegistryEntry*)storage)->previous = storage;
				*(void**)&((RegistryEntry*)storage)->next = storage;
				OSUnlockMutex(anchor);
				next = storage;
			} else {
				*(void**)&((RegistryEntry*)storage)->next = next;
				*(void**)&((RegistryEntry*)storage)->previous = *(void**)&((RegistryEntry*)next)->previous;
				*(void**)&((RegistryEntry*)next)->previous = storage;
				*(void**)(*(char**)(storage + kRegistryPreviousOffset) + kRegistryNextOffset) = storage;
				OSUnlockMutex(anchor);
			}
			AsNameRegistry801C(base)->freeNodes = next;
		}
	}
	// Phase 2: detach one free entry and initialize its key and borrowed/owned name.
	node = AsNameRegistry801C(base)->freeNodes;
	void* anchor2 = base + kRegistryLockOffset;
	OSLockMutex(anchor2);
	void* prev = *(void**)&((RegistryEntry*)node)->previous;
	void* next = *(void**)&((RegistryEntry*)node)->next;
	*(void**)&((RegistryEntry*)next)->previous = *(void**)&((RegistryEntry*)node)->previous;
	*(void**)(*(char**)((char*)node + kRegistryPreviousOffset) + kRegistryNextOffset) = *(void**)&((RegistryEntry*)node)->next;
	void* newHead = *(void**)&((RegistryEntry*)node)->next;
	if (newHead == node) newHead = 0;
	OSUnlockMutex(anchor2);
	AsNameRegistry801C(base)->freeNodes = newHead;
	*(int*)node = *key;
	*((unsigned char*)node + 8) = copyName;
	if (copyName) {
		unsigned int nameLength = strlen(name) + 1;
		char* copy = (char*)appMallocUni__FUl(nameLength);
		*(char**)((char*)node + 4) = copy;
		strcpy(copy, name);
	} else *(const char**)((char*)node + 4) = name;
	// Phase 3: prepend to the hash bucket, then splice into the circular live list.
	if (name != 0) {
		void* anchor3 = base + kRegistryLockOffset;
		OSLockMutex(anchor3);
		int nameLength = (int)strlen(name);
		unsigned int bucketIndex = (((unsigned char)name[0] & kRegistryFirstCharMask) | ((unsigned char)name[nameLength > 0 ? nameLength - 1 : 0] & kRegistryLastCharMask) | (nameLength << kRegistryLengthShift)) & AsNameRegistry801C(base)->hashMask;
		OSUnlockMutex(anchor3);
		OSLockMutex(anchor3);
		void** bucket = (void**)(*(void***)&AsNameRegistry801C(base)->buckets) + bucketIndex;
		*(void**)&((RegistryEntry*)node)->hashNext = *bucket;
		*(void**)((*(char**)&AsNameRegistry801C(base)->buckets) + bucketIndex * 4) = node;
		OSUnlockMutex(anchor3);
	}
	void* first = *(void**)base;
	void* anchor4 = base + kRegistryLockOffset;
	OSLockMutex(anchor4);
	if (first == 0) {
		*(void**)&((RegistryEntry*)node)->previous = node;
		*(void**)&((RegistryEntry*)node)->next = node;
		OSUnlockMutex(anchor4);
	} else {
		*(void**)&((RegistryEntry*)node)->next = first;
		*(void**)&((RegistryEntry*)node)->previous = *(void**)&((RegistryEntry*)first)->previous;
		*(void**)&((RegistryEntry*)first)->previous = node;
		*(void**)(*(char**)((char*)node + kRegistryPreviousOffset) + kRegistryNextOffset) = node;
		OSUnlockMutex(anchor4);
		node = first;
	}
	*(void**)base = node;
}

// Hash a name and return its matching entry key while holding the registry lock.
// Return the stored 32-bit key through the pointer ABI, not a pointer into the registry.
// Zero means absent (and cannot distinguish an entry whose stored key is zero).
void* NameRegistry_FindByName(void* registry, const char* name) {
	RegistryLockGuard lock0(AsNameRegistry801C(registry)->lock);
	if (name == 0 || AsNameRegistry801C(registry)->buckets == 0) return 0;
	unsigned int bucketIndex;
	{
		RegistryLockGuard lock1(AsNameRegistry801C(registry)->lock);
		int nameLength = (int)strlen(name);
		unsigned char first = (unsigned char)name[0];
		unsigned char last = (unsigned char)name[nameLength > 0 ? nameLength - 1 : 0];
		bucketIndex = (((first & kRegistryFirstCharMask) | (last & kRegistryLastCharMask)) | (nameLength << kRegistryLengthShift)) & AsNameRegistry801C(registry)->hashMask;
	}
	RegistryEntry* node = ((RegistryEntry**)AsNameRegistry801C(registry)->buckets)[bucketIndex];
	while (node != 0) {
		if (strcmp(node->name, name) == 0) {
			void* result = *(void**)node;
			return result;
		}
		node = ((RegistryEntry*)node)->hashNext;
	}
	return 0;
}

// Return the address of a mapped integer, inserting the default value if the key is absent.
// The returned value slot belongs to the tree; it is not the integer encoded as a pointer.
void* CourseMaterialMap_GetOrInsertValue(void* arg0, int* key) {
	void* parent;
	unsigned char insertLeft;
	unsigned char beforeFirst;
	void* result = CourseMaterialMap_FindInsertionPosition(arg0, key, &parent, &insertLeft, &beforeFirst);
	void* node = parent;
	if (node == 0 || ((RegistryTreeNode*)node)->key < *key) {
		int value[2] = {*key, s_nameRegistryMapDefaultValue};
		void* inserted = FxSprayTree_InsertNode(arg0, (int*)result, insertLeft, beforeFirst, value);
		node = &((RegistryTreeNode*)inserted)->key;
	} else node = &((RegistryTreeNode*)node)->key;
	return (char*)node + 4;
}

// Release the vector buffer and optionally free its wrapper, preserving volatile size-reset reads.
void* dtor_801C2A88(void* self, int flag) {
    return ::std::__vector_deleting_dtor((Elem_801C2A88Vector*)self, flag);
}

// Insert all bytes of a small-string or heap-string value at the requested destination position.
// position carries the destination character offset through the 32-bit pointer ABI.
void CourseModelString_InsertString(void* destination, void* position, void* source) {
	unsigned int* node = (unsigned int*)source;
	unsigned char* begin;
	unsigned int count;
	if ((node[0] >> 31) == 0) {
		begin = (unsigned char*)source + 1;
		count = *(unsigned char*)source & 0x7F;
	} else {
		begin = (unsigned char*)node[2];
		count = node[1];
	}
	unsigned char local;
	volatile struct { unsigned char c; unsigned char d[3]; } src;
	local = src.c;
	String_ReplaceRange(destination, position, 0, begin, begin + count, &local);
}

// Save a tree iterator and advance it to its in-order successor.
// oldIterator receives a copy; iterator must refer to a live node with a valid parent chain.
void CourseModelTreeIterator_PostIncrement(void** oldIterator, void** iterator, void* unusedTag) {
	*oldIterator = *iterator;
	void* node = ((RegistryTreeNode*)*iterator)->right;
	if (node != 0) {
		while (*(void**)node != 0) node = ((RegistryTreeNode*)node)->left;
		*iterator = node;
	} else {
		void* cursor;
		void* parent;
		// Original 0x801C25F4: walk up to the next left-child ancestor.
		while ((cursor = *iterator,
		        parent = (void*)((unsigned int)((RegistryTreeNode*)cursor)->parentAndColor & ~1u),
		        cursor != *(void**)parent)) {
			*iterator = parent;
		}
		*iterator = parent;
	}
}

// Find the parent and insertion side for a material key, recording the predecessor at or below it.
// predecessor is the last visited node with key <= requested key (not necessarily equal).
// beforeFirst stays true only when the search never takes a right branch.
void* CourseMaterialMap_FindInsertionPosition(void* tree, int* key, void** predecessor, unsigned char* insertLeft, unsigned char* beforeFirst) {
	*predecessor = 0;
	void* parent = (char*)tree;
	parent = (char*)parent + 4;
	RegistryTreeNode* node = ((RegistryTree*)tree)->root;
	*insertLeft = 1;
	*beforeFirst = 1;
	while (node != 0) {
		parent = node;
		if (*key < ((RegistryTreeNode*)node)->key) {
			node = node->left;
			*insertLeft = 1;
		} else {
			*predecessor = node;
			node = ((RegistryTreeNode*)node)->right;
			*insertLeft = 0;
			*beforeFirst = 0;
		}
	}
	return parent;
}

// Empty allocator cleanup hook; no resources are owned.
void CourseModelAllocator_EmptyCleanup() {}

struct Vec801C1AC0 {
	void** data;
	unsigned int len;
	unsigned int cap;
	// Release backing storage only; pointer payloads are not deleted.
	~Vec801C1AC0() { if (data) appFreeUni__FPv(data); }
};

struct PtrAlloc801C1AC0 {
	char value; char pad[3];
};
struct PtrAllocation801C1AC0 {
	char* pointer; char* allocator;
	// Guard an exception message while constructing its shared owner.
	PtrAllocation801C1AC0(char* p, char* a) : pointer(p), allocator(a) {}
	// Free the exception message unless transferred to its shared owner.
	~PtrAllocation801C1AC0() { if (pointer) __dla__FPv(pointer); }
};
struct PtrControlBase801C1AC0 {
	void* table; unsigned strong, weak;
	// Initialize reference counts and the shared character deleter.
	PtrControlBase801C1AC0() { strong = 1; weak = 1; table = (void*)s_sharedCharDeleterVtable; }
};
struct PtrControl801C1AC0 : PtrControlBase801C1AC0 {
	char* pointer;
	// Attach the allocated message to its shared control block.
	PtrControl801C1AC0(char* p) { pointer = p; }
};
struct PtrErrorBase801C1AC0 {
	void* table;
	// Install the logic-error base table before constructing its payload.
	PtrErrorBase801C1AC0() { table = (void*)__vt__Q23std11logic_error; }
	// Destroy the exception base without deleting this storage.
	~PtrErrorBase801C1AC0() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : PtrErrorBase801C1AC0 {
	char* text; void* control;
	// Copy the vector length-error message into guarded shared storage.
	length_error() {
		const char* message = courseModelResourceStrings;
		PtrAlloc801C1AC0 allocator;
		allocator.value = 0;
		unsigned n = strlen(message) + 1;
		text = (char*)__nwa__FUl(2 * n - n);
		{
			char* p = text;
			PtrAllocation801C1AC0 allocation(p, &allocator.value);
			control = new PtrControl801C1AC0(p);
			allocation.pointer = 0;
		}
		strcpy(text, message);
		table = (void*)__vt__Q23std12length_error;
	}
    DECOMP_CT_MEMBER(length_error)
};
}
// Construct and throw the vector length error with exception-safe ownership of its message.
inline void PtrThrow801C1AC0() { {
    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
    std::length_error* error = (std::length_error*)storage.bytes;
    error->__ct();
    exception_runtime::__throw(exception_runtime::courseModelResourceStrings + kLengthErrorTypeOffset, error,
        (void*)exception_runtime::__dt__Q23std12length_errorFv);
} }
struct PtrVector801C1AC0 { void** volatile data; unsigned size, capacity; };
struct PtrBuffer801C1AC0 {
	void** data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
	// Track a pending allocation until the vector ownership swap commits.
	PtrBuffer801C1AC0(unsigned* p) : size(0), data(0), capacity(0), owner(p), offset(0) {}
	// Free the backing block retained after a growth commit or failure.
	~PtrBuffer801C1AC0() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Select the larger growth request by reference.
inline const unsigned& PtrVectorMax801C1AC0(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

#pragma push
#pragma use_lmw_stmw on
// Append a pointer, growing and transferring the backing buffer when capacity is exhausted.
// Copies the pointer value only; neither this vector nor its buffer guard deletes pointees.
void CourseModelPointerVector_Append(void* ptr, void** value) {
    PtrVector801C1AC0* v = (PtrVector801C1AC0*)ptr;
    unsigned int size = v->size;

    if (size < v->capacity) {
        int next = 1 + size;
        *(volatile unsigned*)&v->size = next;
        v->data[next - 1] = *value;
    } else { unsigned int count;
        if (kPointerVectorMax - v->capacity < 1) PtrThrow801C1AC0();
        unsigned needed = (1 + v->size) - v->capacity;
        PtrBuffer801C1AC0 temp(&v->capacity);
        unsigned capacity = v->capacity;
        if (needed > kPointerVectorMax - capacity) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::courseModelResourceStrings + kLengthErrorTypeOffset, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        if (capacity < kPointerVectorSmallGrowthLimit) {
            capacity += PtrVectorMax801C1AC0((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < kPointerVectorMediumGrowthLimit) {
            capacity += PtrVectorMax801C1AC0((((1 + capacity))) / 2, needed);
        } else capacity = 0x3FFFFFFF;
        if (capacity > kPointerVectorMax) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::courseModelResourceStrings + kLengthErrorTypeOffset, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        // Build the new tail first, then relocate old slots; the guard owns new storage.
        void** data = (void**)appMallocDummy__FUl(capacity * 4);
        unsigned int oldSize = v->size;
        temp.offset = oldSize;
        temp.data = data;

        (((oldSize + data)))[temp.size++] = *value;
        void** oldData = v->data;
        count = ((char*)((((v->size + oldData)))) - (char*)oldData) / 4;
        temp.capacity = capacity;

        int offset = oldSize - count;
        temp.offset = offset;
        int bytes;
        bytes = ((count * 4));
        memcpy(((data + offset)), oldData, bytes);
        memset(oldData, 0, bytes);
        // Commit the new buffer and give the guard the old allocation to free on exit.
        unsigned int newSize = (temp.size + count);
        void** oldData2 = v->data;
        void** newData = temp.data;
        unsigned int oldCap = v->capacity;
        temp.size = newSize;
        v->size = 0;
        temp.data = oldData2;
        unsigned int newCap = temp.capacity;
        v->capacity = newCap;
        temp.capacity = oldCap;
        v->data = newData;
        v->size = newSize;
        temp.size = 0;

    }
}
#pragma pop

// Initialize the empty tree root, count and sentinel iterator.
void CourseMaterialMap_InitEmpty(unsigned int* arg0) {
	arg0[0] = 0;
	arg0[1] = 0;
	((RegistryTree*)arg0)->first = (RegistryTreeNode*)&((RegistryTree*)arg0)->root;
}

// Empty iterator cleanup hook; no resources are owned.
void CourseModelIterator_EmptyCleanup() {}

// Return the current pointer-slot address and advance the iterator by one pointer.
void* CourseModelPointerIterator_PostIncrement(void** arg0) {
	void* value = *arg0;
	*arg0 = (char*)value + 4;
	return value;
}

// Return the node stored in the iterator.
void* CourseModelTreeIterator_GetNode(void** arg0) {
	return *arg0;
}

// Copy an iterator node pointer.
void CourseModelTreeIterator_Assign(void** arg0, void** arg1) {
	*arg0 = *arg1;
}

// Return the node stored in the second iterator adapter.
void* CourseModelTreeIterator_GetNodeAdapter(void** arg0) {
	return *arg0;
}

// Return the pointer-slot iterator value.
void* CourseModelPointerIterator_Get(void** arg0) {
	return *arg0;
}

// Initialize an iterator from the tree's first-node slot.
void CourseModelTreeIterator_FromFirstNode(void** arg0, void** arg1) {
	*arg0 = arg1[3];
}

// Copy a tree iterator node pointer.
void CourseModelTreeIterator_Copy(void** arg0, void** arg1) {
	*arg0 = *arg1;
}

// Return the key/value payload address of the iterator's tree node.
void* CourseModelTreeIterator_GetPayload(void** arg0) {
	return &((RegistryTreeNode*)*arg0)->key;
}

// Return the end address of a pointer vector.
void* CourseModelPointerVector_End(void** arg0) {
	return (char*)*arg0 + *(unsigned int*)((char*)arg0 + 4) * 4;
}

// Report whether the shadow-manager singleton exists.
int ShadowManager_IsCreated() {
	return g_shadowManager != 0;
}

// Initialize an empty byte vector.
void CourseModelByteBuffer_Init(int* arg0) {
	arg0[0] = 0;
	arg0[1] = 0;
	arg0[2] = 0;
}

// Resize a byte vector, zeroing appended bytes and preserving existing contents across growth.
// size is bytes. Shrinking changes logical size only; old capacity and bytes are retained.
void CourseModelByteBuffer_ResizeZeroFilled(void** arg0, unsigned int size) {
	unsigned int* self = (unsigned int*)arg0;
	unsigned int oldSize = self[1];
	if (size > oldSize) {
		unsigned int add = size - oldSize;
		unsigned int cap = self[2];
		if (add <= cap && oldSize <= cap - add) {
			memset((char*)self[0] + oldSize, 0, add);
			self[1] = oldSize + add;
			return;
		}
		unsigned int diff = size - self[2];
		unsigned int oldCap = self[2];
		if (diff > kByteVectorMax - oldCap) THROW_LEN_801C1AC0();
		if (oldCap >= kByteVectorSmallGrowthLimit) {
			if (oldCap >= kByteVectorMediumGrowthLimit) {
			}
		}
		Vec801C1AC0 __tmp;
		__tmp.data = 0;
		__tmp.len = 0;
		__tmp.cap = 0;
		// The volatile locals below mirror the compiler-expanded std::vector bookkeeping; they must stay to match.
		unsigned int* volatile __p = &self[2];
		volatile unsigned int __d = 0;

		if (diff > kByteVectorMax - oldCap) THROW_LEN_801C1AC0();

		unsigned int newCap;
		if (oldCap < kByteVectorSmallGrowthLimit) {
			unsigned int g = (oldCap + 1) * 3 / 5;
			unsigned int& sel = (g > diff) ? g : diff;
			newCap = oldCap + sel;
		} else if (oldCap < kByteVectorMediumGrowthLimit) {
			unsigned int g = (oldCap + 1) / 2;
			unsigned int& sel = (g > diff) ? g : diff;
			newCap = oldCap + sel;
		} else {
			newCap = kByteVectorMax;
		}

		// Initialize the appended range, copy the prefix, then exchange ownership.
		char* newData = (char*)appMallocDummy__FUl(newCap);
		__d = self[1];
		__tmp.data = (void**)newData;
		__tmp.cap = newCap;
		memset(newData + self[1] + __tmp.len, 0, add);
		__tmp.len = __tmp.len + add;
		char* oldData = (char*)self[0];
		char* oldFinish = oldData + self[1];
		unsigned int oldLen = (unsigned int)(oldFinish - oldData);
		__d = self[1] - oldLen;
		memcpy(newData + (self[1] - oldLen), oldData, oldLen);
		memset(oldData, 0, oldLen);
		__tmp.len = __tmp.len + oldLen;
		self[1] = 0;
		__tmp.data = (void**)oldData;
		self[2] = newCap;
		__tmp.cap = oldCap;
		self[0] = (unsigned int)newData;
		self[1] = __tmp.len;
		__tmp.len = 0;
	} else if (size < oldSize) {
		self[1] = self[1] - (oldSize - size);
	}
}

// Initialize an empty vector of fixed-size course records.
void CourseModelRecordVector_InitEmpty(unsigned int* arg0) {
	arg0[0] = 0;
	arg0[1] = 0;
	arg0[2] = 0;
}

// Copy all six words of a course-model record without changing their load/store order.
void CourseBounds_CopySixWords(unsigned int* arg0, unsigned int* arg1) { unsigned int e = arg1[4];  int d = arg1[3]; 
	unsigned int c = arg1[2];

	unsigned int b = arg1[1];

	int f = arg1[5];
 arg0[0] = (arg1[0]);
 arg0[1] = b;
	arg0[2] = c;

	arg0[3] = d;
 arg0[4] = e;
 arg0[5] = f;
}

// Reset the byte-vector size, retaining the original volatile read sequence.
void CourseModelByteBuffer_ClearSize(unsigned int* arg0) {
	volatile unsigned int* p = (volatile unsigned int*)&arg0[1];
	*p = *p - *p;
}

// Return the byte-vector address at the requested byte index.
void* CourseModelByteBuffer_GetElement(void** arg0, int arg1) {
	return (char*)*arg0 + arg1;
}

// One 0x88-byte course-model record (contents not yet identified; copied and zero-filled as a block).
struct CourseModelRecord { unsigned int w[34]; };

// Scratch for growing the record vector: holds the new storage while it is filled, then the old block for freeing.
struct RecordVectorGrowthScratch {
	void* data;
	unsigned int len;
	unsigned int cap;
	// Free record-vector backing storage after growth transfers or abandons it.
	~RecordVectorGrowthScratch() { if (data) appFreeUni__FPv(data); }
};

// Resize a vector of 0x88-byte course records, zero-initializing new records.
// size/capacity count records. The six-word copy helper above is a different record format.
void CourseModelRecordVector_ResizeZeroFilled(void** selfArg, unsigned int size) {
	unsigned int* self = (unsigned int*)selfArg;
	unsigned int oldSize = self[1];
	if (size > oldSize) {
		unsigned int add = size - oldSize;
		unsigned int cap = self[2];
		if (add <= cap && oldSize <= cap - add) {
			CourseModelRecord* dst = (CourseModelRecord*)((char*)self[0] + oldSize * 0x88);
			do {
				CourseModelRecord e1 = {};
				*dst = e1;
				self[1] = self[1] + 1;
				dst = dst + 1;
			} while (--add);
			return;
		}
		unsigned int oldCap = self[2];
		if ((self[1] + add) - self[2] > kRecordVectorMax - self[2]) THROW_LEN_801C1AC0();
		if (oldCap >= kRecordVectorSmallGrowthLimit) {
			if (oldCap >= kRecordVectorMediumGrowthLimit) {
			}
		}
		RecordVectorGrowthScratch tmp;
		tmp.data = 0;
		tmp.len = 0;
		tmp.cap = 0;
		// The volatile locals below mirror the compiler-expanded std::vector bookkeeping; they must stay to match.
		unsigned int* volatile __p = &self[2];
		volatile unsigned int __d = 0;
		unsigned int required = (self[1] + add) - self[2];
		if (required > kRecordVectorMax - oldCap) THROW_LEN_801C1AC0();
		unsigned int newCap;
		if (oldCap < kRecordVectorSmallGrowthLimit) {
			unsigned int g = (oldCap + 1) * 3 / 5;
			unsigned int& sel = (g < required) ? required : g;
			newCap = oldCap + sel;
		} else if (oldCap < kRecordVectorMediumGrowthLimit) {
			unsigned int g = (oldCap + 1) / 2;
			unsigned int& sel = (g < required) ? required : g;
			newCap = oldCap + sel;
		} else {
			newCap = kRecordVectorMax;
		}
		if (newCap > kRecordVectorMax) THROW_LEN_801C1AC0();
		// Construct zero records at the tail before copying the retained prefix backwards.
		CourseModelRecord* newData = (CourseModelRecord*)appMallocDummy__FUl(newCap * 0x88);
		__d = self[1];
		tmp.data = newData;
		tmp.cap = newCap;
		{
			unsigned int n = add;
			CourseModelRecord* dst = (CourseModelRecord*)((char*)newData + (self[1] + tmp.len) * 0x88);
			do {
				CourseModelRecord e2 = {};
				*dst = e2;
				tmp.len = tmp.len + 1;
				dst = dst + 1;
			} while (--n);
		}
		{
			CourseModelRecord* first = (CourseModelRecord*)*selfArg;
			CourseModelRecord* src = (CourseModelRecord*)((char*)first + self[1] * 0x88);
			CourseModelRecord* dst = (CourseModelRecord*)((char*)newData + self[1] * 0x88);
			while (src > first) {
				dst = dst - 1;
				src = src - 1;
				if (dst) *dst = *src;
				__d = __d - 1;
				tmp.len = tmp.len + 1;
			}
		}
		// Commit only after all requested records exist; tmp frees the old backing block.
		void* oldBegin = *selfArg;
		self[1] = 0;
		tmp.data = oldBegin;
		self[2] = tmp.cap;
		tmp.cap = oldCap;
		self[0] = (unsigned int)newData;
		self[1] = tmp.len;
		tmp.len = 0;
	}
	else if (size < oldSize) {
		self[1] = oldSize - (oldSize - size);
	}
}

} // extern "C"

typedef unsigned long new_size_t;
#include "game/exception.h"
#include "game/types.h"
#include "classes/CSharedDoc.h"
// Translation unit 0x801BF838-0x801BFDC4 (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma exceptions on
#pragma use_lmw_stmw on
#pragma no_register_save_helpers on
// Construct a record in caller-owned storage without allocating.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {

// Structural views only: callers in this unit do not establish a gameplay subsystem.
// The three payload words are copied verbatim, never interpreted as IDs/pointers.
// Address names are retained until producers/consumers identify their meaning.
// Modification boundary: Append owns only its contiguous record buffer; it never
// releases any of the three words as pointers. Reset mutates four in-place Vec3f
// values and one byte; it neither allocates nor touches the intervening prefix.
// The integer setter and default query do not establish a relationship to either
// record storage or reset vectors; do not infer a common gameplay owner from adjacency.
struct fn_801BF838_Data {
	char baseObject[0x10];
	int value;
};

struct fn_801BFD5C_Data {
	unsigned char stateFlags;
	char pad01[0x27];
	Vec3f vectors[4]; // Four reset vectors; their individual roles are not established.
};

void Vec3_Zero(void* object);
void* appMallocDummy__FUl(unsigned int size);
void appFreeUni__FPv(void* object);

struct fn_801BF840_Element {
	unsigned int firstWord;
	unsigned int secondWord;
	unsigned int thirdWord;
	// Copy the opaque three-word payload in its original load/store order.
	fn_801BF840_Element(const fn_801BF840_Element& x) {
	 unsigned int first = x.firstWord, second = x.secondWord;
	 firstWord = first;
	 unsigned int third = x.thirdWord;
	 secondWord = second;
	 thirdWord = third;
	}
};

// Readable aliases preserve the original type identities used by exception records.
typedef fn_801BF838_Data IntegerPropertyOwnerView;
typedef fn_801BFD5C_Data FourVectorStateView;
typedef fn_801BF840_Element TripleWordRecord;

struct fn_801BF840_Vector {
	char ownerPrefix[0x14];
	fn_801BF840_Element* begin;
	unsigned int size;
	unsigned int capacity;
};

typedef fn_801BF840_Vector TripleRecordStorage;

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], courseModelResourceStrings[];
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv(void*, int);

struct ErrorOwner {
 void* ptr;
 char* flag;
 // Release error text if shared ownership was not established.
 ~ErrorOwner() { if (ptr) __dla__FPv(ptr); }
};
struct ErrorRef {
 void* vt;
 int strong, weak;
 void* ptr;
 // Take ownership of the allocated exception text with one strong/weak reference.
 ErrorRef(void* p) : strong(1), weak(1) { vt = s_sharedCharDeleterVtable; ptr = p; }
};
namespace std {
struct logic_error : exception {
    // Install the runtime exception vtable during construction.
    void SetTable(const void* vt) { *(const void**)this = vt; }

 char* text;
 ErrorRef* ref;
 // Build a shared, null-terminated runtime error message.
 logic_error(const char* s) {
  SetTable(__vt__Q23std11logic_error);
  char flag;
  unsigned int n = strlen(s) + 1;
  char* allocated = (char*)__nwa__FUl(n * 2 - n);
  text = allocated;
  {
   ErrorOwner owner = {allocated, &flag};
   ref = new ErrorRef(allocated);
   owner.ptr = 0;
  }
  strcpy(text, s);
 }

    ~logic_error();
};
}
} // extern "C"
namespace std {
struct length_error : logic_error {
 // Specialize the shared logic-error message as a length error.
 length_error(const char* s) : logic_error(s) { SetTable(__vt__Q23std12length_error); }
 ~length_error();
};
}
extern "C" {
// Throw the vector-capacity exception with the module message.
inline void lengthError() { throw std::length_error(courseModelResourceStrings); }
struct Reallocation {
 TripleWordRecord* begin;
 unsigned int size;
 unsigned int capacity;
 unsigned int* oldCapacity;
 unsigned int remaining;
 // Guard a replacement buffer until it is exchanged with the live vector.
 Reallocation(unsigned int* capacityStorage) : begin(0), size(0), capacity(0), oldCapacity(capacityStorage), remaining(0) {}
 // Free whichever allocation remains guarded after commit or unwinding.
 ~Reallocation() { if (begin) { size = 0; appFreeUni__FPv(begin); } }
};
// Return the larger growth requirement by reference.
inline const unsigned int& maximum(const unsigned int& firstWord, const unsigned int& secondWord) { return firstWord < secondWord ? secondWord : firstWord; }

enum TripleVectorLimits {
    kMaxTripleRecords = 0x15555555u, // UINT_MAX / sizeof(three-word record)
    kTripleThreeFifthsGrowthLimit = 0x071c71c7u,
    kTripleHalfGrowthLimit = 0x0e38e38eu
};

// Append a three-word record, growing the array with exception-safe ownership transfer.
void TripleRecordVector_Append(TripleRecordStorage* self, const TripleWordRecord* value) {
	// Phase 1: append in place when capacity permits; value may alias an old record.
	if (self->size < self->capacity) {
		TripleWordRecord* end = self->begin + self->size;
		new (end) TripleWordRecord(*value);
		++self->size;
		return;
	}

	// Phase 2: validate and choose capacity in records, then guard the new allocation.
	if (kMaxTripleRecords - self->capacity < 1) lengthError();
	unsigned int additionalCapacity = self->size + 1 - self->capacity;
	Reallocation replacement(&self->capacity);
	unsigned int newCapacity = self->capacity;
	if (additionalCapacity > kMaxTripleRecords - newCapacity) lengthError();
	if (newCapacity < kTripleThreeFifthsGrowthLimit) {
		newCapacity += maximum((newCapacity + 1) * 3 / 5, additionalCapacity);
	} else if (newCapacity < kTripleHalfGrowthLimit) {
		newCapacity += maximum((newCapacity + 1) / 2, additionalCapacity);
	} else {
		newCapacity = kMaxTripleRecords;
	}

	if (newCapacity > kMaxTripleRecords) lengthError();
	replacement.begin = (TripleWordRecord*)appMallocDummy__FUl(newCapacity * sizeof(TripleWordRecord));
	replacement.capacity = newCapacity;
	// Phase 3: construct the appended value BEFORE relocating old records (alias-safe).
	replacement.remaining = self->size;
	TripleWordRecord* destination = replacement.begin + self->size + replacement.size;
	for (int count = 1; count != 0; --count, ++destination) {
	 new (destination) TripleWordRecord(*value);
	 ++replacement.size;
	}
	// Relocate backwards; remaining/size track the partially constructed buffer.
	TripleWordRecord* source = self->begin + self->size;
	TripleWordRecord* first = self->begin;
	TripleWordRecord* destinationEnd = replacement.begin + replacement.remaining;
	while ((unsigned int)source > (unsigned int)first) {
		--source;
		--destinationEnd;
		new (destinationEnd) TripleWordRecord(*source);
		--replacement.remaining;
		++replacement.size;
	}

	// Phase 4: exchange allocations; replacement now owns and frees the old buffer.
	TripleWordRecord* previousData = self->begin;
	self->size = 0;
	unsigned int oldCapacity = self->capacity;
	self->capacity = replacement.capacity;
	replacement.capacity = oldCapacity;
	self->begin = replacement.begin;
	replacement.begin = previousData;
	self->size = replacement.size;
	replacement.size = 0;
}

// Clear all four vectors and reset the state byte.
FourVectorStateView* FourVectorState_Reset(FourVectorStateView* object) {
	Vec3_Zero(&object->vectors[0]);
	Vec3_Zero(&object->vectors[1]);
	Vec3_Zero(&object->vectors[2]);
	Vec3_Zero(&object->vectors[3]);
	object->stateFlags = 0;
	return object;
}

// Set the integer property stored after the base-object prefix.
void Object_SetIntegerProperty10(IntegerPropertyOwnerView* data, int value) {
	data->value = value;
}

// Report the default unsupported/false result.
int DefaultQuery_ReturnZero() {
	return 0;
}

extern CSharedDoc* g_sharedDoc;
// Inline name storage lies in a currently unpublished portion of the shared header.
struct SharedCourseResourceView { char prefix[0x9E4]; char courseResourceName[0x20]; };

// Return the inline course resource name in the shared document.
char* SharedDoc_GetCourseResourceName() {
	return ((SharedCourseResourceView*)g_sharedDoc)->courseResourceName;
}

} // extern "C"

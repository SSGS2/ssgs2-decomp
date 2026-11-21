// Translation unit 0x80297160-0x8029736C (5 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

unsigned short* wcsstr(unsigned short*, const void*);
unsigned int wcslen(void*);
void WString_ReserveCapacity(void*, unsigned int);
void WString_ReplaceRange(void*, int, int, void*, void*, unsigned char*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

// Storage contracts used here (32-bit, big-endian PowerPC): a WString is 12
// bytes. Word 0 bit 31 selects heap ownership; heap data is at +8, otherwise
// characters are inline. Lengths count 16-bit code units, not bytes.
// WStringArray_dtor's container has data at +0, live count at +4 and a logical
// starting element index at +16. It is NOT the three-word vector initialized
// by WStringVector_InitEmpty; do not reuse that initializer for this object.
enum { kWStringBytes = 12, kWideCharacterBytes = 2 };

// Destroy the live string range, free its backing array, optionally delete self.
// Owns both heap-backed element strings and the array; returns self, possibly freed.
void* WStringArray_dtor(void* self, int shouldDelete) {
	if (self) {
		int count = *(unsigned int*)((char*)self + 4);

		// Phase 1: walk the live range backward from origin+count, releasing
		// only heap strings. Inline characters are part of the array itself.
		char* elementCursor;
		char* arrayBase = *(char**)self;

		char* firstLiveElement = arrayBase + (*(unsigned int*)((char*)self + 16)) * kWStringBytes;
		elementCursor = firstLiveElement + count * kWStringBytes;
		while ((unsigned)elementCursor > (unsigned)firstLiveElement) {
			elementCursor -= kWStringBytes;
			if (elementCursor) {
				if ((*(unsigned int*)elementCursor >> 31) != 0) {
					appFreeUni__FPv(*(void**)(elementCursor + 8));
				}
			}
		}
		*(unsigned int*)((char*)self + 4) = 0;
		// Phase 2: base-container cleanup sees count==0, so its preserved
		// element loop does no further work; release the backing allocation.
		if (self) {
			if (*(char**)self != 0) {
				int remainingElements = *(unsigned int*)((char*)self + 4);

				arrayBase = *(char**)self;
				unsigned int unusedCountCopy = remainingElements;
				elementCursor = arrayBase + (*(unsigned int*)((char*)self + 4)) * kWStringBytes;
				*(unsigned int*)((char*)self + 4) = 0;
				while (remainingElements) {
					elementCursor = elementCursor - kWStringBytes;
					if (elementCursor) {
						if ((*(unsigned int*)elementCursor >> 31) != 0) {
							appFreeUni__FPv(*(void**)(elementCursor + 8));
						}
					}
					remainingElements = remainingElements - 1;
				}
				appFreeUni__FPv(*(void**)((char*)self));
			}
		}
		if (shouldDelete > 0) __dl__FPv(self);
	}
	return self;
}

// Construct an owning copy of a required NUL-terminated 16-bit string; return self.
// self is fresh 12-byte storage; source is borrowed only for this call.
void* WString_ConstructFromWChars(void* self, void* source) {
	unsigned int* words = (unsigned int*)self;
	words[0] = 0;
	words[1] = 0;
	words[2] = 0;
	unsigned int characterCount = wcslen(source);
	WString_ReserveCapacity(self, characterCount);
	// Preserve the original stateless allocator token copy, including its
	// unspecified byte. It is not string contents or a terminator value.
	unsigned char allocatorStorage[8];
	allocatorStorage[4] = allocatorStorage[0];
	WString_ReplaceRange(self, 0, 0, source, (char*)source + characterCount * kWideCharacterBytes, allocatorStorage + 4);
	return self;
}

// Find the first 16-bit substring; return a borrowed match pointer or null.
// Both inputs are required and NUL-terminated. This wrapper is a tail call.
unsigned short* WString_FindSubstring(unsigned short* haystack, const void* needle) {
	return wcsstr(haystack, needle);
}

// Initialize a fresh {data,count,capacity} vector to empty; allocates nothing.
// Existing contents would be leaked: this is construction, not clear/destruction.
void WStringVector_InitEmpty(void* self) {
	unsigned int* words = (unsigned int*)self;
	words[0] = 0;
	words[1] = 0;
	words[2] = 0;
}

// Free the owned allocation at self+4, then optionally delete self; return self.
// The allocation's payload type is unresolved; no element destructor is called.
void* dtor_8029730C(void* self, int shouldDelete) {
	if (self != 0) {
		void* resource = *(void**)((char*)self + 4);
		if (resource != 0) {
			appFreeUni__FPv(resource);
		}
		if (shouldDelete > 0) {
			__dl__FPv(self);
		}
	}
	return self;
}

} // extern "C"

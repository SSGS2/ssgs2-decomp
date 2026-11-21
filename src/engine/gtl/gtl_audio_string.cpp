// GTL byte strings used by audio: storage lifetime, reserve, and range replacement.
// API/mod lookup: AudioString_dtor releases backing storage; ReserveCapacity changes
// storage without truncation; ReplaceRange is the edit/append/erase primitive.
// MWStringStorage.h supplies the 12-byte Wii ABI; audioStringErrorStrings and
// s_audioString*Terminator are shared diagnostics/terminators, not tuning values.
// Representation (native big-endian Wii words): byte 0 bit 7 selects heap form.
// Inline: byte 0 low 7 bits = character length, bytes 1..11 = storage INCLUDING
// NUL (10 usable characters). Heap: word 0 = high-bit tag | capacity INCLUDING
// NUL, word 1 = character length, word 2 = owned data. No Unicode conversion.
// Heap bytes use gtl_Audio_Alloc/Free; exception-message bytes use new[]/delete[].
// Borrowed character pointers are invalidated by successful reserve/replacement.
// No gameplay tuning knobs. A capacity mod must preserve space for NUL in both
// forms and test 0/10/11 characters, self-insertion, erase-to-empty and exceptions.
// Translation unit 0x8040470C-0x80404D8C (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/MWStringStorage.h"
#include <decomp/cw_string_guard.h>
#include "sdk_protos.h"

extern "C" {
void* memcpy(void*, const void*, unsigned int);
void gtl_Audio_Free(void*);
void __dl__FPv(void*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void* gtl_Audio_Alloc(unsigned int);
extern unsigned char s_sharedCharDeleterVtable[];
struct AudioStringErrorPool {
    char reserveLengthError[0x23];
    char lengthErrorType[0x37];
    char replaceOutOfRange[0x1B];
    char outOfRangeType[0x37];
    char replaceLengthError[0x1B];
    char padding[5];
};
typedef char AudioStringErrorPool_size_check[(sizeof(AudioStringErrorPool) == 0xCC) ? 1 : -1];
extern const AudioStringErrorPool audioStringErrorStrings;
extern unsigned char s_audioStringReallocationTerminator;
extern unsigned char s_audioStringInPlaceTerminator;
void __dt__Q23std12length_errorFv(void*);
void __dt__Q23std12out_of_rangeFv(void*);
extern void* __vt__Q23std11logic_error[4];
extern void* __vt__Q23std12length_error[4];
extern void* __vt__Q23std12out_of_range[4];

struct StringRep {
    void* vtable;
    int refs;
    int referenceControlWord08;
    char* data;
};

enum StringStorageLimits {
    // kInlineCapacity is storage bytes, not the maximum character length.
    kInlineCapacity = 11, kAllocationAlignment = 16,
    kHeapFlag = 0x80000000u, kCapacityMask = 0x7fffffffu, kMaxStringLength = 0x7ffffffeu,
    kInlineHeapFlag = 0x80, kInlineLengthMask = 0x7f
};

// Destroy heap-backed string storage; inline strings own no separate allocation.
void* AudioString_dtor(void* self, int deleteStorage) {
    if (self) {
        unsigned int* flags = &AsMWStringStorage(self)->flags;
        if ((*flags >> 31) != 0)
            gtl_Audio_Free((void*)AsMWStringStorage(self)->data);
        if (deleteStorage > 0)
            __dl__FPv(self);
    }
    return self;
}

// Resize storage without truncating contents; failure to shrink leaves the string intact.
// self is required, initialized 12-byte storage; requested is character capacity
// excluding NUL, clamped upward to current length. >0x7ffffffe throws length_error
// before changing self. Heap growth allocation exceptions propagate; shrink
// allocation exceptions are swallowed. The allocator must return valid storage
// or throw (null is unchecked). Successful moves free the prior heap allocation.
void gtl_String_ReserveCapacity(char* self, unsigned int requested) {
    if (requested > kMaxStringLength) {
        struct Guard { char* first; ~Guard() { if (first) __dla__FPv(first); } } guard = { 0 };
        struct Error { void* vtable; char* message; void* control; } error;
        guard.first = (char*)new char[strlen(audioStringErrorStrings.reserveLengthError) + 1];
        error.vtable = __vt__Q23std11logic_error;
        error.message = guard.first;
        StringRep* control = (StringRep*)__nw__FUl(0x10);
        if (control) {
            control->vtable = (void*)s_sharedCharDeleterVtable;
            control->refs = control->referenceControlWord08 = 1;
            control->data = guard.first;
        }
        error.control = control;
        strcpy(guard.first, audioStringErrorStrings.reserveLengthError);
        guard.first = 0;
        error.vtable = __vt__Q23std12length_error;
        __throw(audioStringErrorStrings.lengthErrorType, &error, (void*)__dt__Q23std12length_errorFv);
        return;
    }

// An MSL string keeps its heap flag in bit 31 of word 0.
    bool heap = (*(unsigned char*)self & kInlineHeapFlag) != 0;
    unsigned int size;
    unsigned int oldCapacity;
    if (heap) {
        size = AsMWStringStorage(self)->length;
        oldCapacity = AsMWStringStorage(self)->flags & kCapacityMask;
    } else {
        size = *(unsigned char*)self & kInlineLengthMask;
        oldCapacity = kInlineCapacity;
    }
    if (requested < size) requested = size;
    unsigned int newCapacity = kInlineCapacity;
    if (requested + 1 > kInlineCapacity) newCapacity = (requested + kAllocationAlignment) & ~(kAllocationAlignment - 1u);
    if (newCapacity == oldCapacity) return;

    char* oldData = heap ? AsMWStringStorage(self)->data : self + 1;
    char* data;
    if (newCapacity == kInlineCapacity) data = self + 1;
    else if (newCapacity > oldCapacity) data = (char*)gtl_Audio_Alloc(newCapacity);
    else {
        try {
            data = (char*)gtl_Audio_Alloc(newCapacity);
        } catch (...) {
            return;
        }
    }
    bool newHeap = newCapacity != kInlineCapacity;
    memmove(data, oldData, size);
    data[size] = 0;
    if (heap) gtl_Audio_Free(oldData);
    unsigned int flags = AsMWStringStorage(self)->flags;
    flags = (flags & kCapacityMask) | (newHeap ? kHeapFlag : 0);
    AsMWStringStorage(self)->flags = flags;
    if (!newHeap) {
        *(unsigned char*)self = (unsigned char)size;
    } else {
        AsMWStringStorage(self)->data = data;
        AsMWStringStorage(self)->length = size;
        AsMWStringStorage(self)->flags = (flags & kHeapFlag) | newCapacity;
    }
}

// Replace the bounded range at position, preserving overlapping insertion text with a temporary string.
// self is required initialized storage; position is in [0,length]. removeCount
// is clamped to length-position. [first,last) must be a readable ordered range
// within one allocation (last-first representable); no NUL is required in it.
// It may alias self's character storage as handled by the copy paths below.
// allocatorTag is forwarded only to recursive overlap copies; it is not read or
// retained and does not select the gtl_Audio allocator in this implementation.
// Returns self after publishing new length/data/NUL. Out-of-range position throws
// out_of_range; oversized result throws length_error. Allocation exceptions
// propagate before destination mutation; the allocator must not return null.
// Reallocation copies all borrowed source bytes before freeing old data; the
// in-place path first snapshots a threatened source range, then shifts the tail.
void* gtl_String_ReplaceRange(char* self, unsigned int position, unsigned int removeCount, const char* first, const char* last, const char* allocatorTag) {
    unsigned int flags = AsMWStringStorage(self)->flags;
    bool heap = (flags >> 31) != 0;
    unsigned int length;
    unsigned int capacity;
    char* data;
    if (!heap) {
        length = *(unsigned char*)self & kInlineLengthMask;
        data = self + 1;
        capacity = kInlineCapacity;
    } else {
        length = AsMWStringStorage(self)->length;
        data = AsMWStringStorage(self)->data;
        capacity = flags & kCapacityMask;
    }
    if (position > length) {
        struct Guard { char* first; ~Guard() { if (first) __dla__FPv(first); } } guard = { 0 };
        struct Error { void* vtable; char* message; void* control; } error;
        error.vtable = __vt__Q23std11logic_error;
        error.message = 0;
        guard.first = new char[strlen(audioStringErrorStrings.replaceOutOfRange) + 1];
        error.message = guard.first;
        StringRep* control = (StringRep*)__nw__FUl(0x10);
        if (control) {
            control->vtable = (void*)s_sharedCharDeleterVtable;
            control->refs = control->referenceControlWord08 = 1;
            control->data = guard.first;
        }
        error.control = control;
        strcpy(guard.first, audioStringErrorStrings.replaceOutOfRange);
        guard.first = 0;
        error.vtable = __vt__Q23std12out_of_range;
        __throw(audioStringErrorStrings.outOfRangeType, &error, (void*)__dt__Q23std12out_of_rangeFv);
        return self;
    }
    unsigned int available = length - position;
    unsigned int removeLength = removeCount < available ? removeCount : available;
    unsigned int retainedLength = length - removeLength;
    unsigned int insertLength = (unsigned int)(last - first);
    unsigned int newLength = retainedLength + insertLength;
    if (insertLength > kMaxStringLength || retainedLength > kMaxStringLength - insertLength) {
        struct Guard { char* first; ~Guard() { if (first) __dla__FPv(first); } } guard = { 0 };
        struct Error { void* vtable; char* message; void* control; } error;
        error.vtable = __vt__Q23std11logic_error;
        error.message = 0;
        guard.first = new char[strlen(audioStringErrorStrings.replaceLengthError) + 1];
        error.message = guard.first;
        StringRep* control = (StringRep*)__nw__FUl(0x10);
        if (control) {
            control->vtable = (void*)s_sharedCharDeleterVtable;
            control->refs = control->referenceControlWord08 = 1;
            control->data = guard.first;
        }
        error.control = control;
        strcpy(guard.first, audioStringErrorStrings.replaceLengthError);
        guard.first = 0;
        error.vtable = __vt__Q23std12length_error;
        __throw(audioStringErrorStrings.lengthErrorType, &error, (void*)__dt__Q23std12length_errorFv);
        return self;
    }
    unsigned int newCapacity = capacity;
    if (newLength >= newCapacity) {
        newCapacity = (capacity + (kAllocationAlignment - 1)) & ~(kAllocationAlignment - 1u);
        while (newCapacity < newLength + 1)
            newCapacity = (newCapacity * 2 + (kAllocationAlignment - 1)) & ~(kAllocationAlignment - 1u);
    }
    if (newLength >= capacity) {
        char* fresh = (char*)gtl_Audio_Alloc(newCapacity);
        if (position != 0) memcpy(fresh, data, position);
        if (insertLength != 0) memmove(fresh + position, first, insertLength);
        unsigned int tailAfter = available - removeLength;
        if (tailAfter != 0) memcpy(fresh + position + insertLength, data + position + removeLength, tailAfter);
        fresh[newLength] = s_audioStringReallocationTerminator;
        if (heap) gtl_Audio_Free(data);
        AsMWStringStorage(self)->flags = kHeapFlag | newCapacity;
        AsMWStringStorage(self)->length = newLength;
        AsMWStringStorage(self)->data = fresh;
    } else {
        unsigned int tailAfter = available - removeLength;
        // The original copies the inserted range first when it lies inside the string past the insertion point:
        // into a local string (empty, short form) by calling itself (0x80404CF4), so up to 10 characters
        // stays in the local's inline buffer and a longer one gets the usual rounded capacity; the local's heap
        // buffer, if any, is freed after the terminator is written (0x80404D3C, 0x80404D60-0x80404D70).
        unsigned int overlapCopy[3] = { 0, 0, 0 };
        if (tailAfter != 0 && data + position + insertLength < last && last <= data + length) {
            gtl_String_ReplaceRange((char*)overlapCopy, 0, 0, first, last, allocatorTag);
            first = (overlapCopy[0] >> 31) ? (char*)overlapCopy[2] : (char*)overlapCopy + 1;
        }
        if (tailAfter != 0) memmove(data + position + insertLength, data + position + removeLength, tailAfter);
        if (insertLength != 0) memmove(data + position, first, insertLength);
        data[newLength] = s_audioStringInPlaceTerminator;
        if (heap) AsMWStringStorage(self)->length = newLength;
        else {
            *(unsigned char*)self = (unsigned char)((*(unsigned char*)self & kInlineHeapFlag) | newLength);
        }
        if (overlapCopy[0] >> 31) gtl_Audio_Free((void*)overlapCopy[2]);
    }
    return self;
}

} // extern "C"

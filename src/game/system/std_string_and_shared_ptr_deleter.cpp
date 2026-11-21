#define GAME_NSTRING_IMPLEMENTATION
#include "game/string.h"
#include <decomp/cw_bitfields.h>

namespace std {
// Release heap storage; inline strings own no allocation.
basic_string<char, char_traits<char>, app::app_allocator<char> >::~basic_string() {
    if (flags >> 31) appFreeUni__FPv(data);
}
}

// Translation unit 0x80046274-0x80046850 (7 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern int s_charArrayDeleterTypeName;
int strcmp(int a, int b);
void* __dla__FPv(void* p);
void* __dl__FPv(void* p, int n);

struct Obj {
    // Shared-pointer control header precedes the owned character-array pointer at +0x0c.
    char controlBlock[0xC];
    int ownedArray;
};

// Release the character array owned by this shared-pointer deleter.
// Requires a valid deleter; does not clear the pointer or free the control block. Return is delete[]'s value.
void* SharedPtrDeleterChar_DeleteArray(Obj* p) {
    return __dla__FPv((void*)p->ownedArray);
}

// Optionally free the deleter control block.
// Null allowed; n > 0 releases only the wrapper. Returns its original, possibly freed address.
void* SharedPtrDeleterChar_dtor(void* p, int n) {
    if (p != 0 && n > 0) {
        __dl__FPv(p, n);
    }
    return p;
}

// Return the stored deleter only when the requested runtime type name matches.
// requestedType's first word is a C-string address; success returns a borrowed pointer-to-pointer, not the array.
void* SharedPtrDeleterChar_GetDeleter(Obj* self, int* requestedType) {
    int typeMatches = strcmp(*requestedType, s_charArrayDeleterTypeName) == 0;
    if (typeMatches) {
        return &self->ownedArray;
    }
    return 0;
}

void appFreeUni__FPv(void* p);
void MWSharedOwner_ReleaseReference(void* p);

// ABI view type identities are retained; fields and aliases describe ownership.
struct A { void* guardedArray; }; // Owns the guarded char[] until transfer.
struct B { unsigned int flag; int pad; void* f8; }; // Unused string view in this unit.
struct D { int messageAddress; void* messageOwner; }; // Borrowed text address and its shared control block.
struct C { char vtableStorage[4]; D message; }; // Exception vtable followed by message/reference.
typedef A CharacterArrayGuard;
typedef D ExceptionMessageReference;
typedef C LengthErrorStorage;
// A/B/C/D are retained ABI type identities: A owns a char array, B is unused,
// C is exception object storage, D pairs borrowed message text with its owner.

// Release a guarded character array and optionally its guard object.
// Null accepted; n > 0 also frees the guard. Return is the original, possibly freed address.
void* __dt__Q33std6detail16char_array_guardFv(CharacterArrayGuard* p, int n) {
    if (p != 0) {
        if (p->guardedArray != 0) {
            __dla__FPv(p->guardedArray);
        }
        if (n > 0) {
            ((void* (*)(void*))__dl__FPv)(p);
        }
    }
    return p;
}

// Drop the exception message's shared reference; the callee name also appears in locale cleanup.
inline void destroyD(ExceptionMessageReference* d) throw() {
    if (d != 0) {
        if (d != 0 && d->messageOwner != 0) {
            MWSharedOwner_ReleaseReference(d->messageOwner);
        }
    }
}

// Destroy the length-error message reference and optionally free the exception.
// Null accepted; n > 0 frees the exception storage. The message is released through its reference object.
void* __dt__Q23std12length_errorFv(LengthErrorStorage* p, short n) {
    if (p != 0) {
        if (p != 0) {
            destroyD(&p->message);
        }
        if (n > 0) {
            ((void* (*)(void*))__dl__FPv)(p);
        }
    }
    return p;
}

extern char s_stringReallocationTerminatorByte;
extern char s_stringInPlaceTerminatorByte;
void* appMallocDummy__FUl(unsigned n);
void* memcpy(void*, const void*, unsigned);
void* memmove(void*, const void*, unsigned);
void __dt__Q23std12out_of_rangeFv();
void* __dt__Q23std12length_errorFv(struct C*, short);
void __throw(const char* a, void* b, void (*c)()) __attribute__((noreturn));
void* __nwa__FUl(unsigned n);
void* __nw__FUl(unsigned n);
unsigned strlen(const char*);
char* strcpy(char*, const char*);

// 12-byte MSL string, big-endian bitfields: bit 31 selects heap storage.
// Inline: first byte carries tag/length, characters begin at self+1 (11 storage bytes).
// Heap: low 31 flag bits are allocated character bytes, +4 is length, +8 is owned data.
// Capacity includes the terminator; this implementation grows when newLength >= capacity.
struct Str {
    union {
        unsigned flag;
        struct { CW_BITS2( unsigned heapbit:1, unsigned capacity:31) };
        struct { CW_BITS2( unsigned char byteheap:1, unsigned char smallsize:7) };
    };
    unsigned size;
    char* ptr;
};
// Shared exception-string pool: +3 out_of_range message, +0x1e its type name;
// +0x55 length_error message, +0x70 its type name. Keep these offsets tied to this pool.
extern char fileWriterExceptionStrings[];
static const unsigned int kStringMaximumLength = 0x7FFFFFFEu;
static const unsigned int kStringHeapFlag = 0x80000000u;
enum StringStorage { kInlineStringCapacity = 11, kStringAllocationGranularity = 16 };
// 16-byte shared message control block: vtable, two counters initialized to one, owned array.
struct Ref { int vt; int c1; int c2; char* s; };
typedef Ref ExceptionMessageSharedOwner;
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
struct Holder_u80046274 {
    char* p; char* flag;
    // Guard the message allocation until a shared control block takes ownership; flag is retained ABI state.
    Holder_u80046274(char* b, char* f) { p = b; flag = f; }
    // Free only an allocation whose ownership has not been transferred by clearing p.
    ~Holder_u80046274() { if (p) __dla__FPv(p); }
};
// ABI class ErrBase is the logic-error message owner; derived Err/Err2 throw
// out_of_range/length_error. Keep these identities stable for exception relocations.
struct ErrBase {
    void* vt; char* buf; ExceptionMessageSharedOwner* ref;
    // Copy a borrowed message into an array owned by a newly allocated shared control block.
    ErrBase(const char* message) {
        char cleanupMarker __attribute__((aligned(4))) = 0;
        vt = __vt__Q23std11logic_error;
        const char* messageText = message;
        unsigned messageBytes = strlen(messageText) + 1;
        char* messageBuffer = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        buf = messageBuffer;
        {
            Holder_u80046274 bufferGuard(messageBuffer, &cleanupMarker);
            ExceptionMessageSharedOwner* messageReference = (ExceptionMessageSharedOwner*)__nw__FUl(sizeof(ExceptionMessageSharedOwner));
            if (messageReference) { int table = (int)s_sharedCharDeleterVtable; messageReference->c1 = 1; messageReference->c2 = 1; messageReference->vt = table; messageReference->s = messageBuffer; }
            ref = messageReference;
            bufferGuard.p = 0;
        }
        strcpy(buf, messageText);
    }
    // Empty source-level shell; emitted exception destructors above release the message reference.
    ~ErrBase() {}
};
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
struct Err2 : ErrBase {
    // Construct and throw length_error using its runtime type and destructor entry.
    Err2(const char* msg) : ErrBase(msg) { vt = __vt__Q23std12length_error; __throw(fileWriterExceptionStrings + 0x70, this, (void (*)())__dt__Q23std12length_errorFv); }
    // Retain the empty derived cleanup shell used by the matched throw expansion.
    ~Err2() {}
};
struct Err : ErrBase {
    // Construct and throw out_of_range using its runtime type and destructor entry.
    Err(const char* msg) : ErrBase(msg) { vt = __vt__Q23std12out_of_range; __throw(fileWriterExceptionStrings + 0x1e, this, __dt__Q23std12out_of_rangeFv); }
    // Retain the empty derived cleanup shell used by the matched throw expansion.
    ~Err() {}
};

struct Alloc { char c __attribute__((aligned(4))); }; // Allocator token, not read by String_ReplaceRange.
#pragma push
#pragma use_lmw_stmw on
// Replace a bounded string range, preserving overlapping input before moving the suffix.
// Offsets/counts are bytes: erase min(eraseCount, oldLength-replaceOffset), then insert [begin,end).
// Requires a valid string and an ordered, readable insertion range; the source may alias self's data.
// Throws out_of_range for offset > length, length_error above 0x7ffffffe characters; returns self.
// Owns any new heap block, releases the old one after copying, and retains no insertion pointer.
Str* String_ReplaceRange(Str* self, unsigned replaceOffset, unsigned eraseCount, const char* insertBegin, const char* insertEnd, const Alloc& extra) {
    int suffixLength;
    bool usesHeap;

    char* characters;
    unsigned int oldLength;
    int capacity;
    unsigned int erasedLength;
    int insertLength;

    // Select the active representation, keeping length/capacity in character-storage bytes.
    usesHeap = 0 != self->heapbit;
    if (usesHeap == 0) {
        oldLength = self->smallsize;
        characters = (char*)self + 1;
        capacity = kInlineStringCapacity;
    } else {
        // Read the heap descriptor once. The unused declarations below preserve
        // CodeWarrior's local-symbol numbering for Temp's unwind destructor;
        // they have no runtime role and are not alternate string lengths.
        char* heapData = self->ptr;
        char* unusedAbiSlot1;
        unsigned unusedAbiSlot2;
        char* unusedAbiSlot3;
        characters = heapData;
        unsigned unusedAbiSlot4;
        unsigned unusedAbiSlot5;
        unsigned unusedAbiSlot6;
        unsigned unusedAbiSlot7;
        unsigned unusedAbiSlot8;
        unsigned unusedAbiSlot9;
        unsigned unusedAbiSlot10;
        unsigned unusedAbiSlot11;
        oldLength = self->size;
        capacity = self->capacity;
    }
    // Validate offset/maximum length and compute the surviving suffix before modifying storage.
    if (replaceOffset > oldLength) {
        char* exceptionMessage = fileWriterExceptionStrings + 3;
        Err e(exceptionMessage);
    }
    unsigned int sizeValue = oldLength;
    unsigned availableLength = sizeValue - replaceOffset;
    const unsigned* eraseLimit = &eraseCount; if (availableLength < eraseCount) eraseLimit = &availableLength;
    erasedLength = *eraseLimit;
    insertLength = insertEnd - insertBegin;
    if (insertLength > kStringMaximumLength || oldLength - erasedLength > kStringMaximumLength - insertLength) {
        Err2 e(fileWriterExceptionStrings + 0x55);
    }
    __typeof__(oldLength) lengthBeforeErase = oldLength;
    __typeof__(lengthBeforeErase) lengthForResize = lengthBeforeErase;
    unsigned newLength;
    unsigned int takeValue = erasedLength;
    newLength = insertLength + (lengthForResize - erasedLength);
    unsigned int sizeValue2 = oldLength;
    suffixLength = sizeValue2 - (replaceOffset + takeValue);
    if (newLength >= capacity) {
        // Grow in 16-byte multiples until the new characters plus terminator fit.
        // Assemble prefix, insertion, suffix in a separate block so aliases still read old storage.
        int capValue = capacity;
        capacity = (capValue + kStringAllocationGranularity - 1) & ~(kStringAllocationGranularity - 1u);
        while (capacity < 1 + newLength) capacity = (capacity * 2 + kStringAllocationGranularity - 1) & ~(kStringAllocationGranularity - 1u);
        int capValue2 = capacity;
        char* newCharacters = (char*)appMallocDummy__FUl(capValue2);
        if (replaceOffset) memcpy(newCharacters, characters, replaceOffset);
        char* insertionStart = replaceOffset + newCharacters;
        char* insertionDestination = insertionStart;
        memmove(insertionDestination, insertBegin, insertEnd - insertBegin);
        if (suffixLength) memcpy(insertLength + insertionDestination, characters + replaceOffset + erasedLength, suffixLength);
        __typeof__(newLength) terminatorOffset = newLength;
        __typeof__(terminatorOffset) terminatorIndex = terminatorOffset;
        newCharacters[terminatorIndex] = s_stringReallocationTerminatorByte;
        // Commit only after assembling the new terminated contents; inline storage then becomes heap-backed.
        if (usesHeap) appFreeUni__FPv(characters);
        else self->flag |= kStringHeapFlag;
        self->ptr = newCharacters;
        unsigned newsizeValue = newLength;
        self->size = newsizeValue;
        self->capacity = capacity;

    } else {
        // Reuse storage: suffix source starts at replaceOffset+erasedLength,
        // suffix destination at replaceOffset+insertLength. Snapshot insertion bytes
        // only when the code's two range tests say the suffix move can overwrite them.
        // The temporary owns its copy; insertLength stays the original range length.
        struct Temp : Str {
            // Release only the overlap snapshot's heap block at scope exit.
            ~Temp() { if (flag >> 31) appFreeUni__FPv(ptr); }
        };
        Temp overlapCopy; overlapCopy.flag = 0; overlapCopy.size = 0; overlapCopy.ptr = 0;
        if (suffixLength) {
            if (characters + replaceOffset + insertLength < insertEnd) {
                if (insertEnd <= oldLength + characters) {
                    Alloc c; // Retained uninitialized token; the callee never reads its byte.
                    unsigned int snapshotEraseCount = !(overlapCopy.flag >> 31) ? (*(unsigned char*)&overlapCopy & 0x7F) : 0;

                    Alloc copy = c;
                    String_ReplaceRange(&overlapCopy, 0, snapshotEraseCount, insertBegin, insertEnd, copy);
                    insertBegin = (overlapCopy.flag >> 31) ? overlapCopy.ptr : (char*)&overlapCopy + 1;
                }
            }
        }
        // Shift the surviving suffix first, insert from the protected source, then publish length/NUL.
        if (suffixLength) {
            char* suffix = (replaceOffset + characters);
            unsigned int takeValue2 = erasedLength;
            memmove(insertLength + suffix, suffix + takeValue2, suffixLength);
        }
        memmove((replaceOffset + characters), insertBegin, insertLength);
        unsigned newsizeValue2 = newLength;
        characters[newsizeValue2] = s_stringInPlaceTerminatorByte;
        if (usesHeap == 0) self->smallsize = newLength;
        else self->size = newLength;
    }
    return self;
}

#pragma pop
} // extern "C"

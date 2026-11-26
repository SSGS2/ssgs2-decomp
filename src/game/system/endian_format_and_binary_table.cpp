// Endian format strings and binary tables: parses byte-swap patterns such as repeat counts
// and applies them when copying data, plus lookup helpers for fixed-stride binary tables.

#include <decomp/static_init.h>
#include "classes/CSwapEndianFormat.h"
#include "classes/BinaryTableView.h"
#define AsBinaryTable800699F0(p) ((BinaryTableView*)(p))

#include "sdk_protos.h"
void* operator new(size_t);
// Construct into caller-owned storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {

// Format callbacks share this three-word cursor ABI. Measuring uses offset;
// swapping uses dataAddress. Address aliases retain the original integer arithmetic.
// Field roles: `offset` is the running byte count used by the measuring pass;
// `cursor` / `cursorAddress` / `signedCursorAddress` are three views of ONE word, the
// position inside the NUL-terminated format text (pointer for char scans, integer where the
// original did address arithmetic); `dataAddress` is the next byte of the buffer being
// byte-swapped (swapping pass only).
struct EndianFormatCursor {
    unsigned int offset;
    union {
        char* cursor;
        unsigned int cursorAddress;
        int signedCursorAddress;
    };
    unsigned int dataAddress;
};
struct CtypeFacetView {
    unsigned int prefix[2];
    unsigned short* classification;
};
struct LocaleDataView {
    char facetPrefix[0x38];
    CtypeFacetView* ctype;
};
struct BinaryTableHeaderView {
    unsigned int tag, version;
    int entryCount;
};
struct InlineTextNode {
    unsigned int header[2];
    char text[1];
};
// A 12-byte string stores inline bytes starting at byte 1 when bit 31 is clear;
// when set, the last word points to owned heap data and the middle word is its length.
struct SmallString12 {
    unsigned int state, length;
    char* data;
};
struct FormatStringVectorView {
    SmallString12* strings;
    int count, capacity;
    void* owner;
    int constructedOffset;
};
enum {
    kWordBytes = 4,
    kWordMask = 3,
    kDecimalRadix = 10,
    kAsciiDigitZero = '0',
    kCtypeDigit = 8,
    kCtypeCharLimit = 0x100,
    kTableEntriesOffset = 0x20,
    kFormatStringBytes = 12,
    kHeapFlagShift = 31,
    kFormatVectorOffset = (int)&((CSwapEndianFormat*)0)->formatStrings,
    kMaxFormatStrings = 0x15555555U,
    kSmallStringGrowthLimit = 0x71C71C7U,
    kMediumStringGrowthLimit = 0xE38E38EU
};

// Return the text payload immediately after the two-word node header.
void* OffsetStringNode_GetText(void* ptr) {
    return ((InlineTextNode*)ptr)->text;
}

// Set the value forwarded to the context-dependent format descriptor.
int CSwapEndianFormat::SetContext(unsigned int value) {
    context = value;
    return 1;
}

extern float kFloat_6_2831855;
extern float s_twoPi_80068524;
extern float s_initializedTwoPi_80069A7C;

// Initialize this endian-format unit's tau constant.
void EndianFormat_InitializeTau() {
    s_twoPi_80068524 = kFloat_6_2831855;
}

// Resolve a relative payload offset when the table has a data region.
void* Table_ResolveDataOffset(void* ptr, int offset) {
    int baseOffset = AsBinaryTable800699F0(ptr)->dataOffset;
    if (baseOffset != 0)
        return (char*)ptr + (offset + baseOffset);
    return 0;
}

// Address an indexed fixed-stride entry after the 0x20-byte table header.
void* Table_GetEntry(void* ptr, int index) {
    return (char*)ptr + index * AsBinaryTable800699F0(ptr)->entryStride + kTableEntriesOffset;
}

// Initialize the binary-table unit's tau constant.
void BinaryTable_InitializeTau() {
    s_initializedTwoPi_80069A7C = kFloat_6_2831855;
}

// Find the requested delimiter, or return null at a missing input or terminator.
static inline char* find_char(char* s, signed char value) {
    if (s == 0)
        return 0;
    for (; *s != 0; ++s) {
        if (*s == value)
            return s;
    }
    return 0;
}

// Advance the format cursor to one byte after the requested delimiter.
void EndianFormat_SkipPastChar(void* ptr, signed char value) {
    ((EndianFormatCursor*)ptr)->cursor = find_char(((EndianFormatCursor*)ptr)->cursor, value) + 1;
}

// XOR successive string bytes into the four bytes of a 32-bit hash.
unsigned int Hash_XorStringBytes(char* str) {
    unsigned int result = 0;
    int index = 0;
    while (*str != 0) {
        unsigned char ch = (unsigned char)*str;
        ++str;
        ((unsigned char*)&result)[index % 4] ^= ch;
        ++index;
    }
    return result;
}

#pragma inline_depth(0)
#pragma use_lmw_stmw on
struct SearchObject {
    void* header;
    virtual void unused0();
    virtual void unused1();
    virtual int accept(void*, int);
};
// Find the next entry with the requested short key that the virtual predicate accepts.
void* SearchObject_FindNext(void* ptr, short value, int start) {
    int end;
    void* item;
    int index;
    void* header = *(void**)ptr;
    if (header != 0)
        end = ((BinaryTableHeaderView*)header)->entryCount;
    else
        end = 0;
    index = start;
    while (index < end) {
        item = Table_GetEntry(*(void**)ptr, index);
        if (item == 0)
            break;
        if (*(short*)item == value) {
            if (((SearchObject*)ptr)->accept(item, index))
                return item;
        }
        ++index;
    }
    return 0;
}
#pragma use_lmw_stmw reset
#pragma inline_depth(255)

// Align the destination address to a word and consume the format token.
void EndianFormat_AlignSwapDataToWord(void* ptr) {
    unsigned int offset = ((EndianFormatCursor*)ptr)->dataAddress;
    unsigned int remainder = offset & kWordMask;
    if (remainder != 0)
        offset += kWordBytes - remainder;
    ((EndianFormatCursor*)ptr)->dataAddress = offset;
    ((EndianFormatCursor*)ptr)->cursorAddress += 1;
}

// Align the measured size to a word and consume the format token.
void EndianFormat_AlignMeasuredSizeToWord(void* ptr) {
    unsigned int offset = ((EndianFormatCursor*)ptr)->offset;
    unsigned int remainder = offset & kWordMask;
    if (remainder != 0)
        offset += kWordBytes - remainder;
    ((EndianFormatCursor*)ptr)->offset = offset;
    ((EndianFormatCursor*)ptr)->cursorAddress += 1;
}

extern char __vt__17CSwapEndianFormat[];

// Initialize the formatter with an empty pattern vector and four-byte context.
CSwapEndianFormat::CSwapEndianFormat() {
    vtbl = __vt__17CSwapEndianFormat;
    formatStrings = 0;
    formatCount = 0;
    formatCapacity = 0;
    context = 4;
}

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);

// Destroy owned heap strings in reverse order, free vector storage, and optionally delete.
// Equivalent plain C:  for (i = count; i-- > 0;) if (strings[i].state & 0x80000000) free(strings[i].data);
//                      free(strings); count = 0; if (shouldDelete > 0) delete this;
// The repeated `ptr != 0` tests are leftovers of inlined destructor wrappers.
void* __dt__11Vec80121A68Fv(void* ptr, int shouldDelete) {
    if (ptr != 0) {
        if (ptr != 0 && ptr != 0 && ptr != 0) {
            void** storage = (void**)ptr;
            if (*storage != 0) {
                void* entry = 0;
                int count = ((FormatStringVectorView*)ptr)->count;
                entry = (char*)*storage + count * kFormatStringBytes;
                ((FormatStringVectorView*)ptr)->count -= count;
                while (count != 0) {
                    entry = (char*)entry - kFormatStringBytes;
                    if (entry != 0 && (((SmallString12*)entry)->state >> kHeapFlagShift) != 0)
                        appFreeUni__FPv(((SmallString12*)entry)->data);
                    --count;
                }
                appFreeUni__FPv(*storage);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

extern unsigned char s_cLocaleData[];

// Parse a decimal repeat count (default one), align the size and add the repeated byte width.
void EndianFormat_MeasureRepeatedValues(void* ptr, unsigned int alignment) {
    // alignment is both byte width and alignment, and must be nonzero. The token
    // is followed by optional decimal digits; missing digits and explicit 0 mean 1.
    // A terminating nondigit (including NUL) must exist before address wraparound.
    // The negative-address counter expresses that wrap bound, not a string length.
    unsigned int offset = ((EndianFormatCursor*)ptr)->offset;
    if (offset % alignment != 0)
        offset += alignment - offset % alignment;
    ((EndianFormatCursor*)ptr)->offset = offset;

    char* current = (char*)(((EndianFormatCursor*)ptr)->cursorAddress + 1);
    int count = -(int)(unsigned int)current;
    int repeatCount = 0;
    CtypeFacetView* table = ((LocaleDataView*)s_cLocaleData)->ctype;
    while (count-- != 0) {
        unsigned char raw = *current;
        signed char ch = raw;
        int digit = ch;
        int isDigit = (digit < 0 || digit >= kCtypeCharLimit) ? 0 : (table->classification[digit] & kCtypeDigit);
        if (isDigit) {
            int signedValue = (signed char)raw;
            ++current;
            repeatCount = repeatCount * kDecimalRadix + signedValue - kAsciiDigitZero;
        } else {
            break;
        }
    }
    if (repeatCount == 0)
        repeatCount = 1;
    ((EndianFormatCursor*)ptr)->cursor = current;
    ((EndianFormatCursor*)ptr)->offset += repeatCount * alignment;
}

// Parse a repeat count, align the output cursor and byte-swap two- or four-byte values.
void EndianFormat_SwapRepeatedValues(void* ptr, unsigned int alignment) {
    // Same decimal grammar as measurement. Integer-held addresses preserve the
    // 32-bit cursor ABI; count is the distance to address wrap, not available data.
    // Width 1 advances only, 2/4 reverse bytes; caller supplies enough writable data.
    int current = ((EndianFormatCursor*)ptr)->signedCursorAddress + 1;
    unsigned int repeatCount = 0;
    CtypeFacetView* base = ((LocaleDataView*)s_cLocaleData)->ctype;
    int count = -current;
    while (count-- != 0) {
        unsigned char raw = *(unsigned char*)current;
        signed char ch = raw;
        int digit = ch;
        int isDigit;
        if (digit < 0 || digit >= kCtypeCharLimit)
            isDigit = 0;
        else {
            unsigned short* table = base->classification;
            isDigit = table[digit] & kCtypeDigit;
        }
        if (!isDigit)
            break;
        repeatCount = repeatCount * kDecimalRadix + (signed char)raw - kAsciiDigitZero;
        ++current;
    }
    if (repeatCount == 0)
        repeatCount = 1;
    ((EndianFormatCursor*)ptr)->cursorAddress = (unsigned int)current;
    unsigned int offset = ((EndianFormatCursor*)ptr)->dataAddress;
    unsigned int rem = offset % alignment;
    if (rem != 0)
        offset += alignment - rem;
    ((EndianFormatCursor*)ptr)->dataAddress = offset;
    while (repeatCount-- > 0) {
        unsigned char* p = (unsigned char*)((EndianFormatCursor*)ptr)->dataAddress;
        // pangya.iff is little-endian on disc.
        if (alignment == 4) {
            unsigned char savedByte = p[0];
            p[0] = p[3];
            p[3] = savedByte;
            savedByte = p[1];
            p[1] = p[2];
            p[2] = savedByte;
        } else if (alignment == 2) {
            unsigned char savedByte = p[0];
            p[0] = p[1];
            p[1] = savedByte;
        }
        ((EndianFormatCursor*)ptr)->dataAddress += alignment;
    }
}

// Skip ASCII spaces; return null at NUL or null input, otherwise the next token.
static inline char* skip_format_spaces(char* s) {
    if (s == 0)
        return 0;
    while (*s != 0) {
        if (*s == 0x20) {
            ++s;
            continue;
        } else {
            return s;
        }
    }
    return 0;
}

#pragma inline_depth(255)
#pragma use_lmw_stmw on
// Reverse the bit groups described by decimal widths in the previously written byte.
// Widths are read from low bits upward and appended to the output from the left.
// Example: byte 0xE4 = 11|10|01|00, widths "2 2 2 2" -> 00|01|10|11 = 0x1B.
// For a full-byte permutation, widths must each be 1..8 and sum to 8; zero is
// treated as 1. Short sums discard unconsumed high bits; long sums truncate.
// dataAddress must have a preceding writable byte and value must delimit the list.
void EndianFormat_ReverseBitGroups(void* ptr, signed char value) {
    unsigned char remainingBits;
    CtypeFacetView* base;
    unsigned char* buffer;
    unsigned int formatAddressCursor;
    unsigned char reversedGroups;
    EndianFormatCursor* cursor;
    unsigned int formatAddress;
    int isDigit;
    int mask;
    int formatChar;
    unsigned char raw;
    int groupWidth;
    int shiftedByte;

    cursor = (EndianFormatCursor*)ptr;
    reversedGroups = 0;
    formatAddress = cursor->cursorAddress;
    buffer = (unsigned char*)cursor->dataAddress;
    formatAddressCursor = formatAddress + 1;
    remainingBits = buffer[-1];
    formatAddressCursor = (unsigned int)skip_format_spaces((char*)formatAddressCursor);
    base = ((LocaleDataView*)s_cLocaleData)->ctype;
    // Loop shape: each iteration parses one decimal number (the group width) from the
    // format text and moves that many low bits of the source byte (remainingBits) into the
    // low end of the result (reversedGroups), so the first group ends up highest.
    // The two inline isDigit blocks below are one inlined isdigit(): a char outside
    // 0..0xFF counts as "not a digit", otherwise ctype bit kCtypeDigit decides.
    // 0x80068524: consume decimal shift widths until the format ends.
    while (formatAddressCursor != 0) {
        isDigit = 0;
        formatChar = (signed char)*(unsigned char*)formatAddressCursor;
        if (formatChar < 0 || formatChar >= kCtypeCharLimit)
            isDigit = 1;
        if (isDigit != 0)
            isDigit = 0;
        else
            isDigit = base->classification[formatChar] & kCtypeDigit;
        if (isDigit == 0)
            break;
        groupWidth = 0;
        while (formatAddressCursor != 0) {
            raw = *(unsigned char*)formatAddressCursor;
            formatChar = (signed char)raw;
            isDigit = 0;
            if (formatChar < 0 || formatChar >= kCtypeCharLimit)
                isDigit = 1;
            if (isDigit != 0)
                isDigit = 0;
            else
                isDigit = base->classification[formatChar] & kCtypeDigit;
            if (isDigit == 0)
                break;
            groupWidth *= kDecimalRadix;
            groupWidth = groupWidth + (signed char)raw - kAsciiDigitZero;
            ++formatAddressCursor;
        }
        if (groupWidth == 0)
            groupWidth = 1;
        mask = (1 << groupWidth) - 1;
        reversedGroups <<= groupWidth;
        shiftedByte = remainingBits >> groupWidth;
        reversedGroups |= (remainingBits & mask);
        remainingBits = (unsigned char)shiftedByte;
        formatAddressCursor = (unsigned int)skip_format_spaces((char*)formatAddressCursor);
    }
    buffer[-1] = reversedGroups;
    cursor->cursor = find_char(cursor->cursor, value) + 1;
}
#pragma use_lmw_stmw reset
#pragma inline_depth(255)

// Destroy registered format strings and their vector, then optionally delete the formatter.
// A named member rather than a destructor: the caller passes the delete flag explicitly.
// The repeated `!= 0` tests are kept because the original compiles them.
void* CSwapEndianFormat::Destroy(int shouldDelete) {
    unsigned int count;
    unsigned int base;
    if (this != 0) {
        count = formatCount;
        base = (unsigned int)formatStrings;
        vtbl = __vt__17CSwapEndianFormat;
        formatCount = count - count;
        base += count * kFormatStringBytes;
        while (count != 0) {
            base -= kFormatStringBytes;
            if (base != 0 && ((((SmallString12*)base)->state >> kHeapFlagShift) != 0))
                appFreeUni__FPv(((SmallString12*)base)->data);
            --count;
        }
        if ((char*)this + kFormatVectorOffset != 0 && (char*)this + kFormatVectorOffset != 0 &&
            (char*)this + kFormatVectorOffset != 0 && (char*)this + kFormatVectorOffset != 0) {
            if ((unsigned int)formatStrings != 0) {
                count = formatCount;
                formatCount = count - count;
                base = (unsigned int)formatStrings + count * kFormatStringBytes;
                while (count != 0) {
                    base -= kFormatStringBytes;
                    if (base != 0 && ((((SmallString12*)base)->state >> kHeapFlagShift) != 0))
                        appFreeUni__FPv(((SmallString12*)base)->data);
                    --count;
                }
                appFreeUni__FPv((void*)(unsigned int)formatStrings);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(this);
    }
    return this;
}

// Destroy a temporary vector's constructed string range, then release its allocation.
void* FormatStringVector_dtor(void* ptr, int shouldDelete) {
    if (ptr != 0) {
        char* begin = (char*)*(void**)ptr + ((FormatStringVectorView*)ptr)->constructedOffset * kFormatStringBytes;
        char* end = begin + ((FormatStringVectorView*)ptr)->count * kFormatStringBytes;
        while ((unsigned int)end > (unsigned int)begin) {
            end -= kFormatStringBytes;
            if (end != 0 && (((SmallString12*)end)->state >> kHeapFlagShift) != 0)
                appFreeUni__FPv(((SmallString12*)end)->data);
        }
        (*(volatile int*)&((FormatStringVectorView*)ptr)->count) = 0;
        void* storage = *(void**)ptr;
        if (storage != 0) {
            (*(volatile int*)&((FormatStringVectorView*)ptr)->count) = 0;
            volatile int count = 0;
            char* entry = (char*)storage + count * kFormatStringBytes;
            while (count != 0) {
                entry -= kFormatStringBytes;
                if ((((SmallString12*)entry)->state >> kHeapFlagShift) != 0)
                    appFreeUni__FPv(((SmallString12*)entry)->data);
                --count;
            }
            appFreeUni__FPv(storage);
        }
        if (shouldDelete > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

extern unsigned int endianFormatCountDescriptors[];
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
extern unsigned char endianFormatErrorStrings[];
extern unsigned int __vt__Q23std11logic_error[];
extern unsigned int s_sharedCharDeleterVtable[];
extern unsigned int __vt__Q23std12length_error[];
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
extern void __dla__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void __dt__Q23std12length_errorFv();

struct ErrorBuffer {
    char* pointer;
    char* tag;
    // Guard newly allocated exception text until its shared owner is constructed.
    ErrorBuffer(char* p, char* t) : pointer(p), tag(t) {}
    // Free text only if ownership has not been transferred.
    ~ErrorBuffer() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct ErrorNode {
    void* vtable;
    int refs, count;
    char* data;
    // Adopt the exception's character allocation with initial reference counts.
    ErrorNode(char* p) : refs(1), count(1) {
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};

struct ErrorBase {
    void* vtable;
    char* data;
    ErrorNode* node;
    // Copy borrowed error text into a shared, owned exception buffer.
    ErrorBase(const char* message) {
        vtable = __vt__Q23std11logic_error;
        volatile char pad[4];
        pad[0] = 0;
        char tag[4];
        tag[0] = 0;
        unsigned int n = strlen(message) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        {
            char* buffer = data;
            ErrorBuffer guard(buffer, tag);
            node = new ErrorNode(buffer);
            guard.pointer = 0;
        }
        strcpy(data, message);
    }
};
struct LengthError : ErrorBase {
    // Specialize the owning logic-error representation as a length error.
    LengthError(const char* message) : ErrorBase(message) {
        vtable = __vt__Q23std12length_error;
    }
};

#define THROW_LENGTH_ERROR()                                                      \
    do {                                                                          \
        char* message = (char*)endianFormatErrorStrings;                          \
        LengthError exception(message);                                           \
        __throw(message + 0x14, &exception, (void*)__dt__Q23std12length_errorFv); \
    } while (0)

struct LocalBlob : SmallString12 {
    // Release a temporary string's heap representation, if selected by its flag.
    ~LocalBlob() {
        if ((state >> kHeapFlagShift) != 0)
            appFreeUni__FPv(data);
    }
};
struct AllocatorTag {
    char value;
    char padding[3];
};
struct Item {
    unsigned int flags, length, heapDataAddress;
};
struct TempVector {
    SmallString12* begin;
    unsigned int count;
    unsigned int capacity;
    void* owner;
    unsigned int offset;
    // Destroy only the constructed interval, then free this temporary's storage.
    ~TempVector() {
        SmallString12* first = begin + offset;
        SmallString12* end = first + count;
        while ((unsigned int)end > (unsigned int)first) {
            --end;
            if (end != 0 && (end->state >> kHeapFlagShift) != 0)
                appFreeUni__FPv(end->data);
        }
        count = 0;
        if (begin != 0) {
            unsigned int n = count;
            volatile AllocatorTag clearTag;
            clearTag.value = 0;
            *(volatile unsigned int*)&count = n - n;
            SmallString12* entry = begin + n;
            while (n != 0) {
                --entry;
                if (entry != 0 && (entry->state >> kHeapFlagShift) != 0)
                    appFreeUni__FPv(entry->data);
                --n;
            }
            appFreeUni__FPv(begin);
        }
    }
};

struct CountState {
    unsigned int offset;
    char* cursor;
    unsigned int unused;
};
// The descriptor table in the game data holds CodeWarrior member-function records {this adjustment, slot, code
// token} (12 bytes).
typedef void (CountState::*CountCall)(unsigned int);
struct CountDesc {
    char chars[4];
    CountCall call;
    unsigned int arg;
};
struct CountTable {
    CountDesc desc[8];
    // Resolve a token's paired character spellings; unknown tokens return null.
    CountDesc* find(char* cursor) {
        CountDesc* selected = 0;
        for (int j = 0; j < 8; ++j) {
            if (*cursor == desc[j].chars[0] || *cursor == desc[j].chars[1]) {
                selected = &desc[j];
                break;
            }
        }
        return selected;
    }
};
struct WriteState {
    unsigned int offset;
    char* cursor;
    unsigned int dataAddress;
};
typedef void (WriteState::*WriteCall)(unsigned int);
struct WriteDesc {
    char chars[4];
    WriteCall call;
    unsigned int arg;
};
struct WriteTable {
    WriteDesc desc[8];
    // Resolve a token's swap callback; unknown tokens return null.
    WriteDesc* find(char* cursor) {
        WriteDesc* selected = 0;
        for (int j = 0; j < 8; ++j) {
            if (*cursor == desc[j].chars[0] || *cursor == desc[j].chars[1]) {
                selected = &desc[j];
                break;
            }
        }
        return selected;
    }
};
extern WriteTable endianFormatSwapDescriptors;
// Skip ASCII spaces and return null when there is no next token.
static inline char* skip_spaces(char* p) {
    if (p == 0)
        return 0;
    while (*p != 0) {
        if (*p == ' ')
            ++p;
        else
            return p;
    }
    return 0;
}

#pragma inline_depth(255)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
#pragma use_lmw_stmw on
// Locate the next measurement token, or null at the end of the pattern.
static inline char* skip_count_spaces(char* p) {
    if (p == 0)
        return 0;
    for (; *p != 0;) {
        if (*p == ' ') {
            ++p;
            continue;
        }
        return p;
    }
    return 0;
}
// Validate the measured record size and append one owned format string per repetition.
// Grammar skeleton: pattern := (space* token)* space*, terminated by NUL.
// A repeated-value token is <descriptor character><decimal count?>; for a
// descriptor of width 2, "<char>3" measures three aligned halfwords (6 bytes).
// Descriptor character pairs and special-token delimiters come from the external
// eight-entry tables: do not infer them from C type names. A group-list token
// carries space-separated decimal widths, e.g. "2 2 2 2", up to its delimiter.
// Count callbacks advance cursor and offset; write callbacks advance cursor and
// dataAddress with identical alignment. Descriptor 0 receives object->context.
// Final record size rounds up to 4 bytes. Invalid tokens are not checked here:
// callers must supply a recognized, terminated pattern and valid group widths.
// Return 0 on size mismatch; 1 on success (also when iterations <= 0).
// pattern is borrowed only during this call; appended strings own their copies.
int CSwapEndianFormat::RegisterFormat(const char* pattern, int outputSize, int iterations) {
    // Phase 1: measure one record without changing the registered vector.
    void* context = (void*)this->context;
    CountState counter;
    counter.cursor = skip_count_spaces((char*)pattern);
    counter.offset = 0;
    CountTable table = *(CountTable*)endianFormatCountDescriptors;
    table.desc[0].arg = (unsigned int)context;
    char* cur;
    while ((cur = counter.cursor) != 0) {
        int index = 0;
        CountDesc* selected = 0;
        CountDesc* desc = table.desc;
        // 0x80068B98: search the eight format descriptors in order.
        while (index < 8) {
            if (*cur == desc->chars[0] || *cur == desc->chars[1]) {
                selected = &table.desc[index];
                break;
            }
            ++desc;
            ++index;
        }
        CountCall call = selected->call;
        (counter.*call)(selected->arg);
        counter.cursor = skip_count_spaces(counter.cursor);
    }
    int total = (int)counter.offset;
    unsigned int remainder = total & kWordMask;
    if (remainder != 0)
        total += kWordBytes - remainder;
    if (outputSize != total)
        return 0;
    // Phase 2: append independent pattern copies; vector growth transfers old
    // string representations and clears their former slots before old storage frees.
    int iteration = 0;
    if (iterations > 0) {
        for (; iteration < iterations; ++iteration) {
            LocalBlob blob;
            blob.state = 0;
            blob.length = 0;
            blob.data = 0;
            unsigned int len = strlen(pattern);
            String_ReserveCapacity(&blob, len);
            volatile AllocatorTag allocator;
            AllocatorTag flag;
            flag.value = allocator.value;
            String_ReplaceRange(&blob, 0, 0, pattern, pattern + len, &flag.value);
            unsigned int size = formatCount;
            unsigned int capacity = formatCapacity;
            volatile AllocatorTag insertTag;
            insertTag.value = 0;
            if (size < capacity) {
                SmallString12* item = (SmallString12*)((unsigned int)formatStrings + size * kFormatStringBytes);
                if (item != 0) {
                    try {
                        unsigned int state = blob.state;
                        if ((state >> kHeapFlagShift) == 0) {
                            unsigned int length = blob.length;
                            item->state = state;
                            item->length = length;
                            item->data = blob.data;
                        } else {
                            item->state = 0;
                            item->length = 0;
                            item->data = 0;
                            String_ReserveCapacity(item, blob.length);
                            volatile AllocatorTag allocator2;
                            AllocatorTag flag2;
                            flag2.value = allocator2.value;
                            String_ReplaceRange(item, 0, 0, blob.data, blob.data + blob.length, &flag2.value);
                        }
                    } catch (...) {
                        throw;
                    }
                }
                ++formatCount;
            } else {
                volatile AllocatorTag growthTag;
                growthTag.value = 0;
                volatile AllocatorTag checkTag;
                checkTag.value = 0;
                if (kMaxFormatStrings - capacity < 1u)
                    THROW_LENGTH_ERROR();
                unsigned int need = formatCount + 1 - formatCapacity;
                TempVector tmp;
                tmp.begin = 0;
                tmp.count = 0;
                tmp.capacity = 0;
                tmp.owner = &formatCapacity;
                tmp.offset = 0;
                capacity = formatCapacity;
                if (need > kMaxFormatStrings - capacity)
                    THROW_LENGTH_ERROR();
                unsigned int newCapacity;
                if (capacity < kSmallStringGrowthLimit) {
                    unsigned int grow = ((capacity + 1) * 3) / 5;
                    newCapacity = capacity + *(grow < need ? &need : &grow);
                } else if (capacity < kMediumStringGrowthLimit) {
                    unsigned int grow = (capacity + 1) / 2;
                    newCapacity = capacity + *(grow < need ? &need : &grow);
                } else {
                    newCapacity = kMaxFormatStrings;
                }
                volatile AllocatorTag reserveTag;
                reserveTag.value = 0;
                if (newCapacity > kMaxFormatStrings)
                    THROW_LENGTH_ERROR();
                tmp.begin = (SmallString12*)appMallocDummy__FUl(newCapacity * kFormatStringBytes);
                tmp.capacity = newCapacity;
                tmp.offset = formatCount;
                SmallString12* slot = tmp.begin + tmp.offset + tmp.count;
                unsigned int left = 1;
                volatile AllocatorTag constructTag;
                constructTag.value = 0;
                while (left != 0) {
                    if (slot != 0) {
                        try {
                            unsigned int state = blob.state;
                            if ((state >> kHeapFlagShift) == 0) {
                                unsigned int length = blob.length;
                                slot->state = state;
                                slot->length = length;
                                slot->data = blob.data;
                            } else {
                                slot->state = 0;
                                slot->length = 0;
                                slot->data = 0;
                                String_ReserveCapacity(slot, blob.length);
                                volatile AllocatorTag allocator3;
                                AllocatorTag flag3;
                                flag3.value = allocator3.value;
                                String_ReplaceRange(slot, 0, 0, blob.data, blob.data + blob.length, &flag3.value);
                            }
                        } catch (...) {
                            throw;
                        }
                    }
                    --left;
                    ++slot;
                    ++tmp.count;
                }
                SmallString12* oldBegin = (SmallString12*)(unsigned int)formatStrings;
                unsigned int oldSize = formatCount;
                unsigned int moved =
                    (unsigned int)(((char*)(oldBegin + oldSize) - (char*)oldBegin) / kFormatStringBytes);
                volatile AllocatorTag moveTag;
                moveTag.value = 0;
                tmp.offset = tmp.offset - moved;
                memcpy(tmp.begin + tmp.offset, oldBegin, moved * kFormatStringBytes);
                memset(oldBegin, 0, moved * kFormatStringBytes);
                tmp.count = tmp.count + moved;
                oldBegin = (SmallString12*)(unsigned int)formatStrings;
                oldSize = formatCount;
                unsigned int oldCapacity = formatCapacity;
                SmallString12* newBegin = tmp.begin;
                unsigned int newCount = tmp.count;
                tmp.begin = oldBegin;
                volatile AllocatorTag swapTag;
                swapTag.value = 0;
                formatCapacity = tmp.capacity;
                tmp.capacity = oldCapacity;
                formatStrings = newBegin;
                formatCount = newCount;
                tmp.count = oldSize;
            }
        }
    }
    return 1;
}
#pragma use_lmw_stmw reset
#pragma inline_depth(255)

#pragma use_lmw_stmw on
// Validate total size, copy the payload, then execute each stored endian-swap pattern.
int CSwapEndianFormat::CopyAndSwap(void* output, void* input, int outputSize) {
    // Measure every registered record before touching output; mismatch returns 0.
    // input/output are borrowed; memcpy's overlap restrictions apply.
    WriteState writer;
    unsigned int size = 0;
    Item* entry;
    for (entry = (*(Item**)&formatStrings); entry != (*(Item**)&formatStrings) + (*(int*)&formatCount); ++entry) {
        void* context = (void*)this->context;
        char* key = (entry->flags >> kHeapFlagShift) == 0 ? (char*)entry + 1 : (char*)entry->heapDataAddress;
        CountCall call;
        CountState counter;
        counter.cursor = skip_spaces(key);
        counter.offset = 0;
        CountTable table = *(CountTable*)endianFormatCountDescriptors;
        table.desc[0].arg = (unsigned int)context;
        while (counter.cursor != 0) {
            CountDesc* selected = table.find(counter.cursor);
            call = selected->call;
            (counter.*call)(selected->arg);
            counter.cursor = skip_spaces(counter.cursor);
        }
        unsigned int amount = counter.offset;
        unsigned int rem = amount & kWordMask;
        if (rem != 0)
            amount += kWordBytes - rem;
        size += amount;
    }
    if (outputSize != (int)size)
        return 0;
    memcpy(output, input, outputSize);
    for (Item* entry2 = (*(Item**)&formatStrings); entry2 != (*(Item**)&formatStrings) + (*(int*)&formatCount);
         ++entry2) {
        void* context = (void*)this->context;
        char* key = (entry2->flags >> kHeapFlagShift) == 0 ? (char*)entry2 + 1 : (char*)entry2->heapDataAddress;
        writer.dataAddress = (unsigned int)output;
        writer.cursor = skip_spaces(key);
        WriteTable table = endianFormatSwapDescriptors;
        table.desc[0].arg = (unsigned int)context;
        while (writer.cursor != 0) {
            WriteDesc* selected = table.find(writer.cursor);
            WriteCall call = selected->call;
            (writer.*call)(selected->arg);
            writer.cursor = skip_spaces(writer.cursor);
        }
        unsigned int amount = writer.dataAddress;
        unsigned int rem = amount & kWordMask;
        if (rem != 0)
            amount += kWordBytes - rem;
        output = (void*)amount;
    }
    return 1;
}
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FF0", _ctors_u80068524_0, EndianFormat_InitializeTau);
STATIC_INIT_ENTRY(".CRT$XCU80461FF0", _ctors_u80068524_1, BinaryTable_InitializeTau);
#pragma pop

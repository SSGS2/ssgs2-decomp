#include "classes/MWStringStorage.h"
#include "classes/MWStreamBuffer.h"
#include "classes/MWStreamState.h"
#include "sdk_protos.h"
#include "classes/CSerifuEcb.h"
#include <decomp/cw_bitfields.h>

#define AsCSerifuEcb(p) ((CSerifuEcb*)(p))

// Navigation: String_*/StdString_* and MWStream* implement narrow runtime text;
// Text_ExpandTaggedText/MessageTable_ExpandTextByIndex implement game UTF-16 tags.
// Numeric formatting resumes at OwnedFacetString and uses locale byte facets.
// WStringObject is the only mutable wide-string receiver: never pass it through
// MWStringStorage or OwnedFacetString, whose inline data starts one byte earlier.
// The merged boundaries reflect recovered objects; these domains share ABI calls,
// not a character-storage type.
// Byte strings use the storage below; UTF-16 strings use a 12-byte object too,
// but inline characters start at byte TWO and lengths count 16-bit code units.
// Opaque extern declarations are adapted at call sites to preserve the PPC ABI;
// casts do not imply that an integer result or string object is a pointee of that type.
// Metrowerks uses 12-byte small strings: bit 31 selects heap storage;
// otherwise the first byte holds a 7-bit length and characters start at byte one.
enum StreamFormatFlags {
    kBoolAlpha = 0x1,
    kDecimal = 0x2,
    kFixed = 0x4,
    kHexadecimal = 0x8,
    kInternal = 0x10,
    kLeft = 0x20,
    kOctal = 0x40,
    kRight = 0x80,
    kScientific = 0x100,
    kShowBase = 0x200,
    kShowPoint = 0x400,
    kShowPositive = 0x800,
    kUnitBuffer = 0x2000,
    kUppercase = 0x4000,
    kBaseField = 0x4A,
    kAdjustField = 0xB0,
    kFloatField = 0x104
};
enum StreamBufferMode {
    kAppendMode = 1,
    kInputMode = 8,
    kOutputMode = 0x10,
    kInputOutputMode = 0x18
};
enum StreamSeekOrigin {
    kSeekBegin = 1,
    kSeekCurrent = 2,
    kSeekEnd = 4
};
enum StreamStatus {
    kStreamBad = 1,
    kStreamFail = 4,
    kStreamBadOrFail = 5,
    kEndOfFile = -1
};
enum StringStorageLimits {
    kInlineCapacity = 11,
    kInlineLengthMask = 0x7F
};
enum MessageTagKind {
    kUnknownMessageTag = -1,
    kMappedValueTag = 0,
    kDiscardedTag = 1,
    kNamedTextTag = 2
};
enum MessageTagLimits {
    kTagLookupCodeUnits = 256,
    kTagKindCount = 3,
    kMappedTagHandlerCount = 2,
    kCarriageReturn = 13
};

enum MessageTableFormat {
    kIndexedTextTable = 0,
    kNamedTextTable = 1
};

// Additional ios_base fields beyond the prefix described by MWStreamState.h.
struct StreamIosView {
    char base[0x20];
    void* locale;
    void* buffer;
    unsigned int precision;
    unsigned int width;
    unsigned short flags;
    unsigned char state;
    unsigned char exceptions;
    void* tiedStream;
    signed char fill;
};
struct StreamSentryView {
    unsigned char ok, flushing;
    char padding[2];
    void* reference;
};
struct StreamLocaleView {
    void* facets;
    void* reference;
};
struct LocaleFacetArrayView {
    void** entries;
    unsigned int count;
};
struct LocaleReferenceView {
    void* vtable;
    int references;
};
struct StreamVirtualBaseView {
    char slots[0x3C];
    int iosBaseOffset;
};
struct StringStreamView {
    void* vtable;
    char padding04[8];
    void* secondaryVtable;
    char padding10[4];
    char streamBuffer[0x1C];
    StreamLocaleView locale;
    char padding38[8];
    MWStringStorage text;
    char iosBase[1];
};
struct StreamFormattingSnapshot {
    void* stream;
    unsigned int width, precision;
    unsigned short flags;
    unsigned char exceptions;
};
struct NumPunctView {
    void* vtable;
    unsigned int references;
    signed char decimalPoint, thousandsSeparator;
    char padding0A[2];
    MWStringStorage grouping, trueName, falseName;
};
// numpunct owns three adjacent 12-byte strings, destroyed in reverse order.
enum NumPunctStorageOffset {
    kNumPunctGroupingOffset = (int)&((NumPunctView*)0)->grouping,
    kNumPunctTrueNameOffset = (int)&((NumPunctView*)0)->trueName,
    kNumPunctFalseNameOffset = (int)&((NumPunctView*)0)->falseName,
    kNumPunctStringBytes = sizeof(MWStringStorage)
};
struct MessageTableHeader {
    char header[8];
    int entryCount;
};
struct OffsetTableHeader {
    void* data;
    int groupCount;
};
struct MessageTableEntry {
    void* keyOffset;
    void* textOffset;
    void* namedTextOffset;
};
struct MessageResourceView {
    void* vtable;
    void* table;
    void* groups;
    int format;
};
struct ResourceDataView {
    char header[8];
    void* data;
};
struct StreamPositionView {
    int high, low, state;
};
struct InputStreamLocaleView {
    char base[0x1C];
    StreamLocaleView locale;
};
struct StringBufferStorageView {
    MWStreamBuffer buffer;
    MWStringStorage text;
};
enum StringStreamOffsets {
    kStringStreamBufferOffset = (int)&((StringStreamView*)0)->streamBuffer,
    kStringStreamSecondaryOffset = (int)&((StringStreamView*)0)->secondaryVtable,
    kStringStreamLocaleOffset = (int)&((StringStreamView*)0)->locale,
    kStringStreamTextOffset = (int)&((StringStreamView*)0)->text,
    kStringStreamIosBaseOffset = (int)&((StringStreamView*)0)->iosBase
};

extern "C" {

extern char* strcpy(char*, const char*);
extern void* __nwa__FUl(unsigned int);
void* String_ReplaceRepeatedChar(void*, unsigned int, unsigned int, unsigned int, signed char);
extern unsigned int appMallocDummy__FUl(unsigned int);
extern void* memset(void*, int, unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12out_of_rangeFv(void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
struct StringErrorStrings {
    char outOfRange[27];         // 0x0
    char outOfRangeType[55];     // 0x1B
    char lengthError[27];        // 0x52
    char lengthErrorType[55];    // 0x6D
    char reserveLengthError[35]; // 0xA4
};
extern StringErrorStrings stringErrorStrings_AF820;
extern unsigned char s_stdStringRangeGrowTerminator;
extern unsigned char s_stdStringRangeInPlaceTerminator;
extern unsigned char s_gameStringRepeatGrowTerminator;
extern unsigned char s_gameStringRepeatInPlaceTerminator;
extern unsigned char s_stdStringRepeatGrowTerminator;
extern unsigned char s_stdStringRepeatInPlaceTerminator;
extern void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q23std12length_errorFv(void*);

extern void appFreeUni__FPv(void*);
extern void std_ios_base_dtor(void*, int);
extern void OStreamSentry_Ctor(void*, void*);
extern void* OStream_Flush(void*);
extern void MWStream_ThrowFailure(void*, const char*);
extern char iosClearFailureMessage[];
extern char stdNumPutCharVtable[];
extern void* __nw__FUl(unsigned int);
extern void MWLocale_InstallFacet(void*, void*, unsigned int);
extern unsigned int s_streamLocaleFacetId;
extern unsigned int s_localeFacetIdCounter;
extern unsigned int s_ctypeFacetId;

extern void* g_currentSharedContext;
extern void __dl__FPv(void* value);
extern void MWSharedOwner_ReleaseReference(void* value);
extern void StdString_ReserveCapacity(void* value, unsigned int size);
extern void String_ReserveCapacity(void* value, int size);
extern void* StdString_ReplaceRange(void* value, unsigned int, unsigned int, char*, char*, unsigned char*);
extern void String_ReplaceRange(void* value, int, unsigned int, char*, char*, unsigned char*);
extern void* memcpy(void* destination, const void* source, unsigned int size);
extern void* Table_GetEntry(void* value, void* key);
extern void* Table_ResolveDataOffset(void* value, void* key);
extern void* OffsetStringNode_GetText(void* value);
extern int Hash_XorStringBytes(void* value);
extern void* CSerifuEcb_dtor(void* value, int deleteFlag);
extern int strcmp(const char*, const char*);
extern void* OffsetTable_GetEntry(void* value, int index);
extern void MWStream_ThrowFailure(void* value, const char* message);
extern char __vt__14CMsgFormatCtrl[];
extern char __vt__10CSerifuEcb[];
extern char s_messageTagKindTable[];
extern char s_messageValueTagHandlers[];
extern char streamRestoreFailureMessage[];
extern char s_mwInfinityText[4];
extern char s_mwNaNText[4];
extern int __signbitd(double);
extern void MWStream_FormatDoubleFixed(void*, void*, double);
extern int MWStream_FormatDoubleScientific(void*, void*, double);
extern void MWStream_FormatDoubleGeneral(void*, void*, double);
extern void* MWCtypeFacet_Construct(void*, unsigned int, unsigned int, unsigned int);
extern void MWStream_CopyLocale(void*, void*);
extern int MWNumPut_ConvertUnsigned64Digits(void*, unsigned long long, char*, void*, int);
extern int MWNumPut_ConvertUnsigned32Digits(void*, unsigned int, char*, void*, void*, int);

// Tear down string-stream storage and locale references, repairing virtual-base offsets.
void* StringStream_dtor(void* value, int deleteFlag) {
    if (value != 0) {
        unsigned char* base = (unsigned char*)value;
        void* vtable = *(void**)base;
        ((StreamVirtualBaseView*)vtable)->iosBaseOffset = (int)(base + kStringStreamIosBaseOffset) - (int)vtable;
        if (base + kStringStreamBufferOffset != 0) {
            if (base + kStringStreamTextOffset != 0 && ((((StringStreamView*)base)->text.flags >> 31) != 0))
                appFreeUni__FPv(((StringStreamView*)base)->text.data);
            if (base + kStringStreamLocaleOffset != 0 && ((StringStreamView*)base)->locale.reference != 0)
                try {
                    MWSharedOwner_ReleaseReference(((StringStreamView*)base)->locale.reference);
                } catch (...) {
                    __unexpected();
                }
        }
        vtable = *(void**)base;
        ((StreamVirtualBaseView*)vtable)->iosBaseOffset = (int)(base + kStringStreamBufferOffset) - (int)vtable;
        if (base + kStringStreamSecondaryOffset != 0) {
            void* secondary = ((StringStreamView*)base)->secondaryVtable;
            if (secondary != 0)
                ((StreamVirtualBaseView*)secondary)->iosBaseOffset =
                    (int)(base + kStringStreamBufferOffset) - (int)secondary;
        }
        vtable = *(void**)base;
        ((StreamVirtualBaseView*)vtable)->iosBaseOffset = (int)(base + kStringStreamSecondaryOffset) - (int)vtable;
        if ((short)deleteFlag != 0 && base + kStringStreamIosBaseOffset != 0)
            std_ios_base_dtor(base + kStringStreamIosBaseOffset, 0);
        if ((short)deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}

// Prepare an output operation, flushing its tied stream and enforcing exception masks.
void OStreamSentry_Ctor(void* value, void* reference) {
    ((StreamSentryView*)value)->ok = 0;
    ((StreamSentryView*)value)->flushing = 0;
    ((StreamSentryView*)value)->reference = reference;
    void* object = *(void**)reference;
    unsigned char flags = ((StreamIosView*)object)->state;
    if (flags == 0) {
        try {
            if (((StreamIosView*)object)->tiedStream)
                OStream_Flush(((StreamIosView*)object)->tiedStream);
        } catch (...) {
            object = *(void**)reference;
            flags = ((StreamIosView*)object)->state;
            if ((flags & kStreamFail) == 0)
                ((StreamIosView*)object)->state |= kStreamBad;
            object = *(void**)reference;
            if ((((StreamIosView*)object)->exceptions & ((StreamIosView*)object)->state) != 0)
                __throw(0, 0, 0);
            return;
        }
        object = *(void**)reference;
        flags = ((StreamIosView*)object)->state;
        if (flags == 0) {
            ((StreamSentryView*)value)->ok = 1;
            return;
        }
    }
    ((StreamIosView*)object)->state |= kStreamFail;
    if (((StreamIosView*)object)->buffer == 0)
        ((StreamIosView*)object)->state |= kStreamBad;
    if ((((StreamIosView*)object)->exceptions & ((StreamIosView*)object)->state) != 0)
        MWStream_ThrowFailure(object, iosClearFailureMessage);
}

// Read the length from the Metrowerks inline or heap string representation.
static unsigned int fn_string_len(void* s) {
    unsigned int word = *(unsigned int*)s;
    return word >> 31 ? AsMWStringStorage(s)->length : *((unsigned char*)s) & kInlineLengthMask;
}
// Return the inline character area or the owned heap character buffer.
static unsigned char* fn_string_ptr(void* s) {
    return (*(unsigned int*)s >> 31) ? (*(unsigned char**)&AsMWStringStorage(s)->data) : (unsigned char*)s + 1;
}

// Release an unfinished exception-message array if construction unwinds.
struct ReserveArrayGuard {
    char* data;
    unsigned char* allocator;
    // Retain an unfinished message allocation for exception cleanup.
    ReserveArrayGuard(char* p, unsigned char& a) : data(p), allocator(&a) {}
    // Delete message bytes unless ownership was transferred.
    ~ReserveArrayGuard() {
        if (data)
            __dla__FPv(data);
    }
};
}
namespace std {
struct exception {
    void* vt;
    char* text;
    void* ref;
    ~exception();
};
struct logic_error : exception {
    // Copy the exception message and give its allocation a reference-counted deleter.
    logic_error(const char* message) {
        vt = __vt__Q23std11logic_error;
        struct Allocator {
            unsigned char data;
            char padding[3];
        };
        Allocator allocator2;
        volatile Allocator allocator;
        allocator.data = 0;
        allocator2.data = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(n * 2 - n);
        char* p = text;
        {
            ReserveArrayGuard guard(p, allocator2.data);
            struct Block {
                void* vt;
                int a, b;
                char* text;
            };
            Block* block = (Block*)__nw__FUl(sizeof(Block));
            if (block) {
                block->a = 1;
                block->b = 1;
                block->vt = s_sharedCharDeleterVtable;
                block->text = p;
            }
            ref = block;
            guard.data = 0;
        }
        strcpy(text, message);
    }

    ~logic_error();
};
struct length_error : logic_error {
    // Construct a length_error by retagging the completed logic_error base.
    length_error(const char* message) : logic_error(message) {
        vt = __vt__Q23std12length_error;
    }
    ~length_error();
};
struct out_of_range : logic_error {
    // Construct an out_of_range by retagging its completed base.
    out_of_range(const char* message) : logic_error(message) {
        vt = __vt__Q23std12out_of_range;
    }
    ~out_of_range();
};
}
extern "C" {
struct ReserveWordBits {
    CW_BITS2(unsigned int heap : 1, unsigned int capacity : 31)
};
struct ReserveByteBits {
    CW_BITS2(unsigned char heap : 1, unsigned char length : 7)
};
#pragma push
#pragma use_lmw_stmw on
// Resize string capacity while preserving contents; a failed shrink leaves it intact.
// value is an initialized owning byte string. requested counts storage bytes;
// no shrink below length is allowed. Growth allocation exceptions propagate;
// shrink allocation exceptions are swallowed. Borrowed data pointers may move.
void StdString_ReserveCapacity(void* value, unsigned int requested) {
    register unsigned int newCapacity;
    unsigned char* self = (unsigned char*)value;
    if (requested > 0x7ffffffe) {
        throw std::length_error(stringErrorStrings_AF820.reserveLengthError);
    }
    unsigned int length;
    bool oldHeap = ((ReserveWordBits*)self)->heap;
    unsigned int oldCapacity;
    if (!oldHeap) {
        length = *self & kInlineLengthMask;
        oldCapacity = kInlineCapacity;
    } else {
        length = AsMWStringStorage(self)->length;
        oldCapacity = ((ReserveWordBits*)self)->capacity;
    }
    if (requested < length)
        requested = length;
    newCapacity = kInlineCapacity;
    newCapacity = (requested + 1 > kInlineCapacity) ? (requested + 16) & ~15u : newCapacity;
    // Phase 2: choose inline/heap destination and allocate before releasing old bytes.
    if (newCapacity != oldCapacity) {
        unsigned char* newData;
        unsigned char* oldData;
        bool heap;
        if (newCapacity == kInlineCapacity) {
            oldData = (*(unsigned char**)&AsMWStringStorage(self)->data);
            newData = self + 1;
            heap = 0;
        } else {
            if (newCapacity > oldCapacity)
                newData = (unsigned char*)__nw__FUl(newCapacity);
            else {
                try {
                    newData = (unsigned char*)__nw__FUl(newCapacity);
                } catch (...) {
                    return;
                }
            }
            heap = 1;
            if (oldHeap)
                oldData = (*(unsigned char**)&AsMWStringStorage(self)->data);
            else
                oldData = self + 1;
        }
        // Phase 3: preserve text, terminate, release old ownership, publish representation.
        memmove(newData, oldData, length);
        newData[length] = 0;
        if (oldHeap)
            __dl__FPv(oldData);
        ((ReserveWordBits*)self)->heap = heap;
        if (!heap) {
            ((ReserveByteBits*)self)->length = length;
        } else {
            (*(unsigned char**)&AsMWStringStorage(self)->data) = newData;
            AsMWStringStorage(self)->length = length;
            ((ReserveWordBits*)self)->capacity = newCapacity;
        }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Append one output byte, growing the backing string and rebasing get/put pointers.
int MWStringBuf_Overflow(void* value, int input) {
    unsigned char* self = (unsigned char*)value;
    if ((AsMWStreamBuffer(self)->mode & kOutputMode) == 0)
        return -1;
    if (input == -1)
        return 0;
    unsigned int begin = AsMWStreamBuffer(self)->putCurrent;
    unsigned int end = AsMWStreamBuffer(self)->putEnd;
    if (begin < end) {
        *(unsigned char*)begin = (unsigned char)input;
        ++begin;
        AsMWStreamBuffer(self)->putCurrent = begin;
        if (AsMWStreamBuffer(self)->highWater < begin)
            AsMWStreamBuffer(self)->highWater = begin;
        if (AsMWStreamBuffer(self)->mode & kInputMode) {
            if (AsMWStreamBuffer(self)->getEnd < AsMWStreamBuffer(self)->highWater)
                AsMWStreamBuffer(self)->getEnd = AsMWStreamBuffer(self)->highWater;
        }
        return input;
    }
    try {
        // No room left: append the character to the backing string, grow the string to its capacity,
        // then point the put area at the (possibly moved) buffer.
        unsigned int first = AsMWStreamBuffer(self)->putBegin;
        int size = AsMWStreamBuffer(self)->highWater - first;
        int need = begin - first + 1;
        int getOffset = AsMWStreamBuffer(self)->getCurrent - AsMWStreamBuffer(self)->getBegin;
        if (size < need)
            size = need;
        char c = (char)input;
        unsigned char alloc;
        void* text = &((StringBufferStorageView*)self)->text;
        String_ReplaceRange(text, fn_string_len(text), 0, &c, &c + 1, &alloc);
        unsigned int capacity = (*(unsigned int*)text >> 31 ? *(unsigned int*)text & 0x7fffffff : kInlineCapacity) - 1;
        unsigned int length = fn_string_len(text);
        if (capacity > length)
            String_ReplaceRepeatedChar(text, length, 0, capacity - length, 0);
        else
            String_ReplaceRepeatedChar(text, capacity, length - capacity, 0, 0);
        unsigned char* data = fn_string_ptr(text);
        if (AsMWStreamBuffer(self)->mode & kInputMode) {
            AsMWStreamBuffer(self)->getBegin = (unsigned int)data;
            AsMWStreamBuffer(self)->getCurrent = (unsigned int)(data + getOffset);
            AsMWStreamBuffer(self)->getEnd = (unsigned int)(data + size);
        }
        length = fn_string_len(text);
        AsMWStreamBuffer(self)->putBegin = (unsigned int)data;
        AsMWStreamBuffer(self)->putEnd = (unsigned int)(data + length);
        AsMWStreamBuffer(self)->putCurrent = (unsigned int)(data + need);
        AsMWStreamBuffer(self)->highWater = (unsigned int)(data + size);
    } catch (...) {
        return -1;
    }
    return input;
}
#pragma pop
extern void* StdString_ReplaceRepeatedChar(void*, unsigned int, unsigned int, unsigned int, signed char = 0);
// Find a decimal point, returning -1 through offset when the input has none.
// Caller-owned cursors retain this compiler's original register allocation.
static inline void FindDecimalPoint(unsigned char* const& digitData, const unsigned int& n, const int& decimalPoint,
                                    int& offset, unsigned char*& end, unsigned char*& cursor) {
    if (n != 0) {
        end = digitData + n;
        cursor = digitData;
        signed char c = (signed char)decimalPoint;
        for (; cursor < end; ++cursor)
            if (c == (signed char)*cursor) {
                offset = cursor - digitData;
                return;
            }
    }
    offset = -1;
}
#pragma push
#pragma use_lmw_stmw on
// Insert locale separators into the integer portion, walking digits right to left.
void NumericText_InsertGroupingSeparators(int decimalPoint, int separator, void* source, void* result) {
    unsigned char* grouping = (unsigned char*)source;
    unsigned char* digits = (unsigned char*)result;
    int offset;
    unsigned int inlineGrouping, heapDigits;
    inlineGrouping = !(*(unsigned int*)grouping >> 31);
    unsigned int groupingLength =
        inlineGrouping ? (*grouping & kInlineLengthMask) : AsMWStringStorage(grouping)->length;
    if (groupingLength == 0)
        return;
    heapDigits = *(unsigned int*)digits >> 31;
    unsigned int inlineDigits = !heapDigits;
    unsigned int digitLength = inlineDigits ? (*digits & kInlineLengthMask) : AsMWStringStorage(digits)->length;
    if (digitLength <= 1)
        return;
    unsigned char* digitData;
    unsigned int n;
    if (heapDigits == 0) {
        n = *digits & kInlineLengthMask;
        digitData = digits + 1;
    } else {
        digitData = (*(unsigned char**)&AsMWStringStorage(digits)->data);
        n = AsMWStringStorage(digits)->length;
    }

    unsigned char* scanEnd;
    unsigned char* scan;
    FindDecimalPoint(digitData, n, decimalPoint, offset, scanEnd, scan);
    if ((unsigned)(offset + 0x10000) == 0xffff)
        offset = inlineDigits ? (*digits & kInlineLengthMask) : AsMWStringStorage(digits)->length;
    unsigned int groupIndex = 0;
    unsigned int groupSize = *(inlineGrouping ? grouping + 1 : (*(unsigned char**)&AsMWStringStorage(grouping)->data));
    unsigned int digitsInGroup = 0;
    while (--offset != 0) {
        ++digitsInGroup;
        if (digitsInGroup == groupSize) {
            StdString_ReplaceRepeatedChar(result, offset, 0, 1, (signed char)separator);
            unsigned int sh = !(*(unsigned int*)grouping >> 31);
            unsigned int length = sh ? (*grouping & kInlineLengthMask) : AsMWStringStorage(grouping)->length;
            ++groupIndex;
            if (groupIndex < length)
                groupSize = (sh ? grouping + 1 : (*(unsigned char**)&AsMWStringStorage(grouping)->data))[groupIndex];
            digitsInGroup = 0;
        }
    }
}
#pragma pop

// Resolve named text by hash, then confirm the full key to handle collisions.
int MessageTable_FindTextByName(void* messageTable, void* name) {
    int hash = Hash_XorStringBytes(name);
    struct LookupContainer {
        void* head;
        void* vtable;
    } container;
    container.head = AsCSerifuEcb(messageTable)->table;
    container.vtable = __vt__10CSerifuEcb;
    int index = -1;
    for (;;) {
        int size = container.head ? ((MessageTableHeader*)container.head)->entryCount : 0;
        if (index >= size - 1)
            return 0;
        ++index;
        if (index < 0)
            index = 0;
        void* current = Table_GetEntry(container.head, (void*)index);
        if (!current)
            return 0;
        void* node = *(void**)current;
        void* candidate = Table_ResolveDataOffset(container.head, node);
        int matched = *(int*)candidate == hash;
        if (matched) {
            void* keyNode = Table_ResolveDataOffset(container.head, node);
            char* key = keyNode ? (char*)OffsetStringNode_GetText(keyNode) : 0;
            matched = strcmp((char*)name, key) == 0;
        }
        if (matched) {
            return (int)Table_ResolveDataOffset(container.head, ((MessageTableEntry*)current)->namedTextOffset);
        }
    }
}

struct FlushGuard2404 {
    unsigned char ok, flushing;
    void* reference;
    // Prepare the output sentry and tied-stream handling.
    FlushGuard2404(void* p) {
        OStreamSentry_Ctor(this, p);
    }
    // Flush unit-buffered output on scope exit unless already flushing or failed.
    ~FlushGuard2404() {
        unsigned char* object = *(unsigned char**)reference;
        if ((((StreamIosView*)object)->state & 5) == 0 && (AsMWStreamState(object)->flags & kUnitBuffer) &&
            flushing == 0)
            OStream_Flush(reference);
    }
};
struct FlushBuffer2404 {
    virtual void slot0();
    virtual void slot4();
    virtual void slot8();
    virtual void slotC();
    virtual void slot10();
    virtual int sync();
};
// Synchronize the stream buffer and propagate failures through the stream exception mask.
void* OStream_Flush(void* value) {
    FlushGuard2404 local(value);
    local.flushing = 1;
    FlushBuffer2404* buffer = (*(FlushBuffer2404**)&((StreamIosView*)*(void**)value)->buffer);
    if (buffer != 0) {
        int failed = 0;
        try {
            if (buffer->sync() == -1)
                failed = 1;
        } catch (...) {
            ((StreamIosView*)*(void**)value)->state |= kStreamBad;
            if (((StreamIosView*)*(void**)value)->exceptions & 1)
                throw;
        }
        if (failed) {
            unsigned char* object = *(unsigned char**)value;
            ((StreamIosView*)object)->state |= kStreamBad;
            if (((StreamIosView*)object)->buffer == 0)
                ((StreamIosView*)object)->state |= kStreamBad;
            unsigned int exceptions = (*(volatile unsigned char*)&((StreamIosView*)object)->exceptions);
            unsigned int state = ((StreamIosView*)object)->state;
            if (state & exceptions)
                MWStream_ThrowFailure(object, iosClearFailureMessage);
        }
    }
    return value;
}

int MWStringBuf_GetString(void* first, void* second);
int MWStreamBuffer_Read(void* first, void* destination, int length);
int MWStreamBuffer_Write(void* first, void* source, int length);
void MWNumPunct_GetGrouping(void* first, void* second);
int MessageTable_GetTextByIndex(void* first, void* second);
int TextResource_FindNamedEntry(void* first, void* second);
// Twelve-byte mutable UTF-16 string object; distinct from the byte-string ABI.
struct WStringObject {
    unsigned int flags;       // High bit selects heap; inline length is byte0 & 0x7F.
    unsigned int heapLength;  // UTF-16 code units; inactive as metadata in inline form.
    unsigned short* heapData; // Owned only in heap form; inline characters start at +2.
};
int Text_ExpandTaggedText(void* messageTable, WStringObject* output, unsigned short* source, void* allowNamedLookup);
extern void WString_ReplaceRange(void*, unsigned int, unsigned int, unsigned short*, unsigned short*, unsigned char*);

typedef int (*DispatchFn)(void*, void*);

struct DispatchVtable {
    char pad[0x18];
    DispatchFn slot18;
    DispatchFn slot1C;
};

// Default no-op hook used by the stream implementation.
void MWStreamBuffer_NoOpHookA() {}
// Second default no-op hook used by the stream implementation.
void MWStreamBuffer_NoOpHookB() {}
// Initialize a failed stream position: offset -1 and conversion state zero.
void MWStreamPosition_InitInvalid(void* value) {
    ((StreamPositionView*)value)->low = -1;
    ((StreamPositionView*)value)->high = -1;
    ((StreamPositionView*)value)->state = 0;
}
// Reset a stream position to the same failure sentinel.
void MWStreamPosition_ResetInvalid(void* value) {
    ((StreamPositionView*)value)->low = -1;
    ((StreamPositionView*)value)->high = -1;
    ((StreamPositionView*)value)->state = 0;
}
// Default stream synchronization hook reports success.
int MWStreamBuffer_DefaultSync() {
    return 0;
}
// Default stream hook returns zero.
int MWStreamBuffer_DefaultZeroHook() {
    return 0;
}
// Copy the stream locale handle and retain its shared reference count.
void MWStream_CopyLocale(void* value, void* source) {
    void* shared = ((StreamIosView*)source)->locale;
    *(int*)value = (*(int*)&((StreamLocaleView*)shared)->facets);
    void* ref = ((StreamLocaleView*)shared)->reference;
    ((StreamLocaleView*)value)->reference = ref;
    if (ref != 0) {
        ++((LocaleReferenceView*)ref)->references;
    }
}
extern unsigned int s_numPunctCharFacetId;
extern double kDouble_0_StreamNumberFormatting;
extern void MWDecimalDigits_FromDouble(void*, double);
extern unsigned int s_localeFacetIdCounter;
extern char numPunctCharVtable[];
extern char boolTrueNameString[5];
extern char falseKeywordString[6];
extern void StdString_ConstructCString(void*, char*);
extern int StdNumPunct_GetTrueName(void*, void*);
extern int StdNumPunct_GetFalseName(void*, void*);
extern void StdString_ReserveCapacity(void*, unsigned int);
typedef int (*DispatchFn)(void*, void*);
typedef int (*PutBoolFn)(void*, void*, void*, signed char, int);
struct StringRep12 {
    unsigned int word;
    unsigned int length;
    unsigned char* data;
};
// Write a byte directly when space remains, otherwise dispatch to buffer overflow.
static inline int bput_byte(void** pout, signed char c) {
    void* out = *pout;
    unsigned char* next = (*(unsigned char**)&AsMWStreamBuffer(out)->putCurrent);
    unsigned char* end = (*(unsigned char**)&AsMWStreamBuffer(out)->putEnd);
    if (next < end) {
        *next = (unsigned char)c;
        (*(unsigned char**)&AsMWStreamBuffer(out)->putCurrent) = next + 1;
        return *next;
    }
    return (*(int (**)(void*, unsigned char))(*(char**)out + 0x38))(out, (unsigned char)c);
}
// Write fill bytes, marking the output iterator failed after the first write error.
static inline void bput_pad(void** pout, int count, signed char c) {
    for (int i = 0; i < count; ++i) {
        int failed = 0;
        if (*pout != 0) {
            if (bput_byte(pout, c) == -1)
                failed = 1;
        }
        if (failed)
            *pout = 0;
    }
}
// Write a character range, preserving the iterator failure state.
static inline void bput_text(void** pout, unsigned char* data, int count) {
    for (int i = 0; i < count; ++i) {
        signed char c = (signed char)*data;
        ++data;
        int failed = 0;
        if (*pout != 0) {
            if (bput_byte(pout, c) == -1)
                failed = 1;
        }
        if (failed)
            *pout = 0;
    }
}
#pragma push
#pragma use_lmw_stmw on
// Initialize the default facet header and three empty strings before ownership guards are armed.
static inline void InitializeEmptyBoolPunctuation(unsigned char* facetBytes) {
    ((NumPunctView*)facetBytes)->references = 0;
    ((NumPunctView*)facetBytes)->vtable = numPunctCharVtable;
    ((NumPunctView*)facetBytes)->decimalPoint = '.';
    ((NumPunctView*)facetBytes)->thousandsSeparator = ',';
    ((NumPunctView*)facetBytes)->grouping.flags = 0;
    ((NumPunctView*)facetBytes)->grouping.length = 0;
    (*(unsigned int*)&((NumPunctView*)facetBytes)->grouping.data) = 0;
    ((NumPunctView*)facetBytes)->trueName.flags = 0;
    ((NumPunctView*)facetBytes)->trueName.length = 0;
    (*(unsigned int*)&((NumPunctView*)facetBytes)->trueName.data) = 0;
    ((NumPunctView*)facetBytes)->falseName.flags = 0;
    ((NumPunctView*)facetBytes)->falseName.length = 0;
    (*(unsigned int*)&((NumPunctView*)facetBytes)->falseName.data) = 0;
}

// Format a boolean using locale names or numeric output, including width and padding.
// iter points to one stream-buffer pointer; a null returned pointer means output
// failure. source is borrowed ios state; fill is a byte. boolalpha selects localized
// true/false text; otherwise dispatch to the numeric overload at vtable+0x10.
// Width applies to this field and resets to zero only on the text path here.
// Cleanup types below reproduce unwind ownership. To change boolean text policy,
// edit the selected-name phase; to change padding, edit only the emission phase.
// Example with localized trueName="true", width=6, fill='.': right/internal
// alignment emits "..true", left emits "true..". A failed write makes every
// subsequent iterator snapshot null while normal cleanup still runs.
int StdNumPut_PutBool(void* self, void* iter, void* source, signed char fill, int value) {
    // Guard ownership: temporary text -> heap bytes; locale -> shared reference;
    // partially built facet -> its three strings plus facet allocation.
    struct StringCleanup {
        unsigned char* string;
        // Free this completed temporary string only if it owns heap bytes.
        ~StringCleanup() {
            if (string && (*(unsigned int*)string >> 31))
                __dl__FPv((*(void**)&AsMWStringStorage(string)->data));
        }
    };
    struct LocaleCleanup {
        unsigned char* locale;
        // Release the retained locale reference; unexpected release exceptions terminate.
        ~LocaleCleanup() {
            if (locale && ((StreamLocaleView*)locale)->reference) {
                try {
                    MWSharedOwner_ReleaseReference(((StreamLocaleView*)locale)->reference);
                } catch (...) {
                    __unexpected();
                }
            }
        }
    };
    struct FacetCleanup {
        unsigned char* facet;
        // Destroy the partially built facet strings backwards, then free the facet.
        ~FacetCleanup() {
            if (facet) {
                for (int offset = kNumPunctFalseNameOffset; offset >= kNumPunctGroupingOffset;
                     offset -= kNumPunctStringBytes) {
                    if (*(unsigned int*)(facet + offset) >> 31)
                        __dl__FPv(*(void**)(facet + offset + 8));
                }
                __dl__FPv(facet);
            }
        }
    };
    unsigned char* iosBytes = (unsigned char*)source;
    if ((AsMWStreamState(iosBytes)->flags & kBoolAlpha) != 0) {
        // Phase 1: retain the locale and find/install its punctuation facet.
        unsigned char text[0x10];
        unsigned char truenamePart[0x10];
        unsigned char falsenamePart[0x10];
        unsigned char falsename[0x10];
        unsigned char truename[0x10];
        // Volatile iterator snapshots preserve matching stores/lifetimes. They carry
        // buffer/failure state only, and add no alternate formatting behavior.
        void* volatile rightPaddingSnapshot;
        void* volatile leadingPaddingSnapshot;
        void* volatile internalPaddingSnapshot;
        void* volatile textSnapshot;
        void* volatile leftPaddingSnapshot;
        unsigned char localeSnapshot[8];
        ((void (*)(void*, void*))MWStream_CopyLocale)(localeSnapshot, source);
        LocaleCleanup localeCleanup = {localeSnapshot};
        volatile unsigned char localeConstructionMarker = 0;
        if (s_numPunctCharFacetId == 0)
            s_numPunctCharFacetId = ++s_localeFacetIdCounter;
        unsigned int index = s_numPunctCharFacetId;
        void** facetArray = (void**)*(void**)localeSnapshot;
        void* facet = index < ((LocaleFacetArrayView*)facetArray)->count ? ((void**)*facetArray)[index] : 0;
        if (facet == 0) {
            facet = __nw__FUl(0x30);
            if (facet != 0) {
                unsigned char* pendingFacet = (unsigned char*)facet;
                InitializeEmptyBoolPunctuation(pendingFacet);
                // Facet ownership: the guard owns its strings/allocation until disarmed;
                // MWLocale_InstallFacet below then publishes it in the retained locale.
                FacetCleanup facetCleanup = {pendingFacet};
                ((void (*)(void*, char*, signed char))StdString_ConstructCString)(truename, boolTrueNameString, 0);
                StringCleanup trueCleanup = {truename};
                if ((((NumPunctView*)pendingFacet)->trueName.flags >> 31) == 0 &&
                    (*(unsigned int*)truename >> 31) == 0) {
                    *(StringRep12*)(pendingFacet + kNumPunctTrueNameOffset) = *(StringRep12*)truename;
                } else {
                    unsigned int count =
                        (((NumPunctView*)pendingFacet)->trueName.flags >> 31)
                            ? ((NumPunctView*)pendingFacet)->trueName.length
                            : (unsigned int)(pendingFacet[kNumPunctTrueNameOffset] & kInlineLengthMask);
                    unsigned char* data;
                    unsigned int length;
                    if ((*(unsigned int*)truename >> 31) == 0) {
                        data = truename + 1;
                        length = (unsigned int)(truename[0] & kInlineLengthMask);
                    } else {
                        data = (*(unsigned char**)&AsMWStringStorage(truename)->data);
                        length = AsMWStringStorage(truename)->length;
                    }
                    volatile unsigned int allocatorBits;
                    unsigned char alloc = *((volatile unsigned char*)&allocatorBits);
                    StdString_ReplaceRange(pendingFacet + kNumPunctTrueNameOffset, 0, count, (char*)data,
                                           (char*)(data + length), &alloc);
                }
                trueCleanup.string = 0;
                if (*(unsigned int*)truename >> 31)
                    __dl__FPv((*(void**)&AsMWStringStorage(truename)->data));
                ((void (*)(void*, char*, signed char))StdString_ConstructCString)(falsename, falseKeywordString, 0);
                StringCleanup falseCleanup = {falsename};
                if ((((NumPunctView*)pendingFacet)->falseName.flags >> 31) == 0 &&
                    (*(unsigned int*)falsename >> 31) == 0) {
                    *(StringRep12*)(pendingFacet + kNumPunctFalseNameOffset) = *(StringRep12*)falsename;
                } else {
                    unsigned int count =
                        (((NumPunctView*)pendingFacet)->falseName.flags >> 31)
                            ? ((NumPunctView*)pendingFacet)->falseName.length
                            : (unsigned int)(pendingFacet[kNumPunctFalseNameOffset] & kInlineLengthMask);
                    unsigned char* data;
                    unsigned int length;
                    if ((*(unsigned int*)falsename >> 31) == 0) {
                        data = falsename + 1;
                        length = (unsigned int)(falsename[0] & kInlineLengthMask);
                    } else {
                        data = (*(unsigned char**)&AsMWStringStorage(falsename)->data);
                        length = AsMWStringStorage(falsename)->length;
                    }
                    volatile unsigned int allocatorBits;
                    unsigned char alloc = *((volatile unsigned char*)&allocatorBits);
                    StdString_ReplaceRange(pendingFacet + kNumPunctFalseNameOffset, 0, count, (char*)data,
                                           (char*)(data + length), &alloc);
                }
                falseCleanup.string = 0;
                if (*(unsigned int*)falsename >> 31)
                    __dl__FPv((*(void**)&AsMWStringStorage(falsename)->data));
                facetCleanup.facet = 0;
            }
            if (s_numPunctCharFacetId == 0)
                s_numPunctCharFacetId = ++s_localeFacetIdCounter;
            ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(*(void**)localeSnapshot, facet,
                                                                          s_numPunctCharFacetId);
            facet = ((void**)*facetArray)[index];
        }
        // The stream still owns the locale/facet; release only this temporary reference.
        localeCleanup.locale = 0;
        {
            void* cleanup = ((StreamLocaleView*)localeSnapshot)->reference;
            if (cleanup != 0) {
                try {
                    MWSharedOwner_ReleaseReference(cleanup);
                } catch (...) {
                    __unexpected();
                    for (;;) {
                    }
                }
            }
        }
        // Phase 2: copy only the selected localized name into an owned field string.
        signed char truenameValid = 0;
        signed char falsenameValid = 0;
        StringCleanup truePartCleanup = {0};
        StringCleanup falsePartCleanup = {0};
        unsigned char* chosen;
        if (value != 0) {
            StdNumPunct_GetTrueName(truenamePart, facet);
            truePartCleanup.string = truenamePart;
            chosen = truenamePart;
            truenameValid = 1;
        } else {
            StdNumPunct_GetFalseName(falsenamePart, facet);
            falsePartCleanup.string = falsenamePart;
            falsenameValid = 1;
            chosen = falsenamePart;
        }
        if ((*(unsigned int*)chosen >> 31) == 0) {
            *(StringRep12*)text = *(StringRep12*)chosen;
        } else {
            *(unsigned int*)text = 0;
            AsMWStringStorage(text)->length = 0;
            (*(unsigned int*)&AsMWStringStorage(text)->data) = 0;
            StdString_ReserveCapacity(text, AsMWStringStorage(chosen)->length);
            volatile unsigned int allocatorBits;
            unsigned char alloc = *((volatile unsigned char*)&allocatorBits);
            unsigned char* data = (*(unsigned char**)&AsMWStringStorage(chosen)->data);
            unsigned int length = AsMWStringStorage(chosen)->length;
            StdString_ReplaceRange(text, 0, 0, (char*)data, (char*)(data + length), &alloc);
        }
        falsePartCleanup.string = 0;
        if (falsenameValid && (*(unsigned int*)falsenamePart >> 31))
            __dl__FPv((*(void**)&AsMWStringStorage(falsenamePart)->data));
        truePartCleanup.string = 0;
        if (truenameValid && (*(unsigned int*)truenamePart >> 31))
            __dl__FPv((*(void**)&AsMWStringStorage(truenamePart)->data));
        StringCleanup textCleanup = {text};
        int isShort = !(*(unsigned int*)text >> 31);
        unsigned int length = isShort ? (unsigned int)(text[0] & kInlineLengthMask) : *(unsigned int*)(text + 4);
        unsigned char* data = isShort ? text + 1 : *(unsigned char**)(text + 8);
        // Phase 3: pad before text except for left alignment (pad after text).
        // Internal alignment also pads before text: a boolean has no sign prefix.
        int width = (*(int*)&AsMWStreamState(iosBytes)->width);
        volatile void* outputStart = (void*)*(unsigned int*)iter;
        int remaining = 0;
        if (width > (int)length)
            remaining = width - (int)length;
        unsigned short alignment = AsMWStreamState(iosBytes)->flags & kAdjustField;
        if (alignment != kLeft && alignment != kInternal) {
            volatile void* rightPadding = outputStart;
            bput_pad((void**)&rightPadding, remaining, fill);
            rightPaddingSnapshot = (void*)rightPadding;
            outputStart = rightPadding;
        }
        leadingPaddingSnapshot = (void*)outputStart;
        volatile void* afterLeadingPadding = outputStart;
        if (alignment == kInternal) {
            volatile void* internalPadding = afterLeadingPadding;
            bput_pad((void**)&internalPadding, remaining, fill);
            internalPaddingSnapshot = (void*)internalPadding;
            afterLeadingPadding = internalPadding;
        }
        volatile void* afterText = afterLeadingPadding;
        bput_text((void**)&afterText, data, (int)length);
        textSnapshot = (void*)afterText;
        volatile void* outputEnd = afterText;
        if (alignment == kLeft) {
            volatile void* leftPadding = afterText;
            bput_pad((void**)&leftPadding, remaining, fill);
            leftPaddingSnapshot = (void*)leftPadding;
            outputEnd = leftPadding;
        }
        AsMWStreamState(iosBytes)->width = 0;
        textCleanup.string = 0;
        if (*(unsigned int*)text >> 31)
            __dl__FPv(*(void**)(text + 8));
        return (int)outputEnd;
    }
    // Numeric path delegates integer formatting and iterator failure handling.
    unsigned int argument = *(unsigned int*)iter;
    PutBoolFn method = *(PutBoolFn*)(*(char**)self + 0x10);
    return method(self, &argument, source, fill, value);
}
#pragma pop
extern unsigned int s_numPunctCharFacetId;
extern char numPunctCharVtable[];
extern char boolTrueNameString[5];
extern char falseKeywordString[6];
extern void StdString_ConstructCString(void*, char*);
extern unsigned long long __mod2u(unsigned int, unsigned int, unsigned int, unsigned int);
extern unsigned long long __div2u(unsigned int, unsigned int, unsigned int, unsigned int);
#pragma push
#pragma use_lmw_stmw on
// Insert a signed 32-bit value through num_put and record formatting or output failure.
// input carries the integer bits in a pointer-typed ABI word; it is not dereferenced.
// Original dispatch is vtable+0x10 (NumPut_FormatSignedInteger), not +0x0C (PutBool).
// The initial input != 0 only seeds a status temporary; formatting receives input unchanged.
void* MWOStream_InsertSigned32(void* reference, void* input) {
    FlushGuard2404 check(reference);
    try {
        if (check.ok) {
            int failed = input != 0;
            try {
                void* element;
                {
                    struct LocaleReference {
                        void* array;
                        void* cleanup;
                        // Retain the stream locale for the nested formatting operation.
                        LocaleReference(void* owner) {
                            MWStream_CopyLocale(this, owner);
                        }
                        // Release the locale reference when formatting leaves this scope.
                        ~LocaleReference() throw() {
                            if (cleanup)
                                MWSharedOwner_ReleaseReference(cleanup);
                        }
                    } shared(*(void**)reference);
                    if (s_streamLocaleFacetId == 0)
                        s_streamLocaleFacetId = ++s_localeFacetIdCounter;
                    unsigned int index = s_streamLocaleFacetId;
                    void* array = shared.array;
                    element = index < ((LocaleFacetArrayView*)array)->count ? ((void**)*(void**)array)[index] : 0;
                    if (!element) {
                        element = __nw__FUl(8);
                        if (element) {
                            (*(unsigned int*)&((LocaleReferenceView*)element)->references) = 0;
                            *(void**)element = stdNumPutCharVtable;
                        }
                        if (s_streamLocaleFacetId == 0)
                            s_streamLocaleFacetId = ++s_localeFacetIdCounter;
                        MWLocale_InstallFacet(array, element, s_streamLocaleFacetId);
                        element = ((void**)*(void**)array)[index];
                    }
                }
                void* owner = *(void**)reference;
                unsigned int pair[2];
                pair[0] = (*(unsigned int*)&((StreamIosView*)owner)->buffer);
                pair[1] = pair[0];
                signed char flag = ((StreamIosView*)owner)->fill;
                void* vt = *(void**)element;
                unsigned int result = ((unsigned int (*)(void*, unsigned int*, void*, signed char, void*))(
                    ((void**)vt)[4]))(element, pair, owner, flag, input);
                failed = result == 0;
            } catch (...) {
                unsigned char* owner = *(unsigned char**)reference;
                ((StreamIosView*)owner)->state |= kStreamBad;
                owner = *(unsigned char**)reference;
                if (((StreamIosView*)owner)->exceptions & 1)
                    throw;
            }
            if (failed) {
                unsigned char* owner = *(unsigned char**)reference;
                ((StreamIosView*)owner)->state |= kStreamBadOrFail;
                if (((StreamIosView*)owner)->buffer == 0)
                    ((StreamIosView*)owner)->state |= kStreamBad;
                if (((StreamIosView*)owner)->exceptions & ((StreamIosView*)owner)->state)
                    MWStream_ThrowFailure(owner, iosClearFailureMessage);
            }
        }
    } catch (...) {
        check.flushing = 1;
        throw;
    }
    return reference;
}
#pragma pop
// Default underflow hook reports end of file.
int MWStreamBuffer_DefaultUnderflow() {
    return -1;
}
struct VDisp06AC {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int call();
};
// Refill the get area through its virtual hook, then consume one unsigned byte.
int MWStreamBuffer_ConsumeAfterRefill(void* value) {
    int result = ((VDisp06AC*)value)->call();
    if (result == -1) {
        return -1;
    }
    int offset = (*(int*)&AsMWStreamBuffer(value)->getCurrent);
    (*(int*)&AsMWStreamBuffer(value)->getCurrent) = offset + 1;
    return ((unsigned char*)offset)[0];
}
// Seek relative to begin/current/end, rejecting unsupported modes and out-of-range offsets.
void MWStringBuf_SeekOffset(void* output, void* state, long long amount, int operation, unsigned int flags) {
    int* result = (int*)output;
    unsigned char* data = (unsigned char*)state;
    long long limit;
    int adjusted;
    unsigned int mask = flags & kInputOutputMode;
    if ((kInputOutputMode == mask && operation == kSeekCurrent) || mask == 0) {
        result[1] = -1;
        result[0] = -1;
        result[2] = 0;
        return;
    }
    if (((flags & kInputMode) && !(AsMWStreamBuffer(data)->mode & kInputMode)) ||
        ((flags & kOutputMode) && !(AsMWStreamBuffer(data)->mode & kOutputMode))) {
        result[1] = -1;
        result[0] = -1;
        result[2] = 0;
        return;
    }
    if (((flags & kInputMode) && !(*(int*)&AsMWStreamBuffer(data)->getCurrent)) ||
        ((flags & kOutputMode) && !(*(int*)&AsMWStreamBuffer(data)->putCurrent))) {
        result[1] = -1;
        result[0] = -1;
        result[2] = 0;
        return;
    }
    if (AsMWStreamBuffer(data)->highWater < AsMWStreamBuffer(data)->putCurrent)
        AsMWStreamBuffer(data)->highWater = AsMWStreamBuffer(data)->putCurrent;
    if (AsMWStreamBuffer(data)->getEnd && AsMWStreamBuffer(data)->getEnd < AsMWStreamBuffer(data)->highWater)
        AsMWStreamBuffer(data)->getEnd = AsMWStreamBuffer(data)->highWater;
    if (flags & kOutputMode)
        adjusted = (*(int*)&AsMWStreamBuffer(data)->highWater) - (*(int*)&AsMWStreamBuffer(data)->putBegin);
    else
        adjusted = (*(int*)&AsMWStreamBuffer(data)->highWater) - (*(int*)&AsMWStreamBuffer(data)->getBegin);
    long long first;
    limit = adjusted;
    switch (operation) {
    case kSeekBegin:
        first = 0;
        break;
    case kSeekCurrent:
        if (flags & kOutputMode)
            first = (*(int*)&AsMWStreamBuffer(data)->putCurrent) - (*(int*)&AsMWStreamBuffer(data)->putBegin);
        else
            first = (*(int*)&AsMWStreamBuffer(data)->getCurrent) - (*(int*)&AsMWStreamBuffer(data)->getBegin);
        break;
    case kSeekEnd:
        if (flags & kOutputMode)
            first = limit;
        else
            first = (*(int*)&AsMWStreamBuffer(data)->getEnd) - (*(int*)&AsMWStreamBuffer(data)->getBegin);
        break;
    default:
        result[1] = -1;
        result[0] = -1;
        result[2] = 0;
        return;
    }
    long long sum = first + amount;
    if (sum < 0 || sum > limit) {
        result[1] = -1;
        result[0] = -1;
        result[2] = 0;
        return;
    }
    if ((AsMWStreamBuffer(data)->mode & kAppendMode) && (flags & kOutputMode) && sum != limit) {
        result[2] = 0;
        result[1] = -1;
        result[0] = -1;
        return;
    }
    if (flags & kInputMode)
        (*(int*)&AsMWStreamBuffer(data)->getCurrent) = (*(int*)&AsMWStreamBuffer(data)->getBegin) + (int)sum;
    if (flags & kOutputMode)
        (*(int*)&AsMWStreamBuffer(data)->putCurrent) = (*(int*)&AsMWStreamBuffer(data)->putBegin) + (int)sum;
    result[1] = (int)sum;
    result[0] = (int)(sum >> 32);
    result[2] = 0;
}

// Select the smaller erase length by reference.
static inline unsigned int* MinEraseLength(unsigned int* a, unsigned int* b) {
    return *b < *a ? b : a;
}
#pragma push
#pragma use_lmw_stmw on
// Replace a byte range with repeated characters using the game allocator.
// Indices/counts are bytes, eraseCount clamps to the suffix. Returns self; throws
// out_of_range for position>length and length_error on size overflow. The game
// allocator owns new storage. Standard-string variant below uses operator new/delete.
void* String_ReplaceRepeatedChar(void* self, unsigned int position, unsigned int eraseCount, unsigned int repeatCount,
                                 signed char fillCharacter) {
    unsigned char* storage = (unsigned char*)self;
    unsigned int flagWord = *(unsigned int*)storage;
    int usesHeap = flagWord >> 31;
    char* oldData;
    unsigned int capacity;
    unsigned int length;
    if (!usesHeap) {
        oldData = (char*)storage + 1;
        capacity = kInlineCapacity;
        length = *((unsigned char*)storage) & kInlineLengthMask;
    } else {
        oldData = AsMWStringStorage(storage)->data;
        capacity = flagWord & 0x7fffffff;
        length = AsMWStringStorage(storage)->length;
    }
    if (position > length) {
        throw std::out_of_range(stringErrorStrings_AF820.outOfRange);
    }
    unsigned int suffix = length - position;
    unsigned int removedLength = *MinEraseLength(&eraseCount, &suffix);
    unsigned int max = 0x7ffffffe;
    if (repeatCount > max || length - removedLength > max - repeatCount) {
        throw std::length_error(stringErrorStrings_AF820.lengthError);
    }
    unsigned int newLen = length - removedLength + repeatCount;
    if (newLen >= capacity) {
        unsigned int newCap = (capacity + 15) & ~15;
        while (newCap < newLen + 1)
            newCap = (newCap * 2 + 15) & ~15;
        char* data = (char*)appMallocDummy__FUl(newCap);
        if (position)
            memcpy(data, oldData, position);
        memset(data + position, (unsigned char)fillCharacter, repeatCount);
        if (suffix - removedLength)
            memcpy(data + position + repeatCount, oldData + position + removedLength, suffix - removedLength);
        data[newLen] = s_gameStringRepeatGrowTerminator;
        if (usesHeap)
            appFreeUni__FPv(oldData);
        else
            *(unsigned int*)storage |= 0x80000000;
        AsMWStringStorage(storage)->data = data;
        AsMWStringStorage(storage)->length = newLen;
        *(unsigned int*)storage = (*(unsigned int*)storage & 0x80000000) | newCap;
    } else {
        if (suffix - removedLength)
            memmove(oldData + position + repeatCount, oldData + position + removedLength, suffix - removedLength);
        memset(oldData + position, (unsigned char)fillCharacter, repeatCount);
        oldData[newLen] = s_gameStringRepeatInPlaceTerminator;
        if (!usesHeap)
            storage[0] = (storage[0] & 0x80) | (newLen & kInlineLengthMask);
        else
            AsMWStringStorage(storage)->length = newLen;
    }
    return self;
}
#pragma pop

struct InsertTemp {
    unsigned int word;
    unsigned int length;
    char* data;
    // Create an empty owned string to protect overlapping insertion data.
    InsertTemp() {
        word = 0;
        length = 0;
        data = 0;
    }
    // Free the overlap-protection string if it grew onto the heap.
    ~InsertTemp() {
        if (word >> 31)
            __dl__FPv(data);
    }
};
// Clamp an unsigned erase count to the available suffix.
static inline const unsigned int& MinStringRangeLength(const unsigned int& a, const unsigned int& b) {
    return (b < a) ? b : a;
}
#pragma push
#pragma use_lmw_stmw on
// Replace a range, preserving overlapping input and the inline/heap string representation.
// self is initialized owned storage; [first,last) is a borrowed ordered byte range,
// possibly inside the current text. Return self; position>length and excessive
// length throw. allocPlace is ABI allocator-token storage, not a text/fill pointer.
void* StdString_ReplaceRange(void* self, unsigned int position, unsigned int eraseCount, char* first, char* last,
                             unsigned char* allocPlace) {
    unsigned char* storage = (unsigned char*)self;
    unsigned int word = *(unsigned int*)storage;
    int usesHeap = word >> 31;
    unsigned int tail;
    unsigned int newLength;
    char* oldData;
    unsigned int length;
    unsigned int capacity;
    unsigned int removed;
    unsigned int insertLength;
    if (!usesHeap) {
        oldData = (char*)storage + 1;
        capacity = kInlineCapacity;
        length = storage[0] & kInlineLengthMask;
    } else {
        oldData = AsMWStringStorage(storage)->data;
        capacity = word & 0x7fffffff;
        length = AsMWStringStorage(storage)->length;
    }
    if (position > length)
        throw std::out_of_range(stringErrorStrings_AF820.outOfRange);
    unsigned int suffix = length - position;
    removed = MinStringRangeLength(eraseCount, suffix);
    insertLength = (unsigned int)(last - first);
    if (insertLength > 0x7FFFFFFEu || length - removed > 0x7FFFFFFEu - insertLength)
        throw std::length_error(stringErrorStrings_AF820.lengthError);
    newLength = insertLength + (length - removed);
    tail = length - (position + removed);
    // Phase 2: grow/copy into a fresh buffer, or preserve an overlapping insert
    // before shifting the existing suffix in place.
    if (newLength >= capacity) {
        unsigned int newCapacity = (capacity + 15) & ~15u;
        unsigned int limit = newLength + 1;
        while (newCapacity < limit)
            newCapacity = (newCapacity * 2 + 15) & ~15u;
        char* data = (char*)__nw__FUl(newCapacity);
        if (position != 0)
            memcpy(data, oldData, position);
        memmove(data + position, first, insertLength);
        if (tail != 0)
            memcpy((void*)((unsigned int)(data + position) + insertLength),
                   (void*)(removed + (unsigned int)(oldData + position)), tail);
        data[newLength] = s_stdStringRangeGrowTerminator;
        if (usesHeap)
            __dl__FPv(oldData);
        else
            *(unsigned int*)storage |= 0x80000000;
        // Publish ownership only after prefix, insertion, suffix and terminator exist.
        AsMWStringStorage(storage)->data = data;
        AsMWStringStorage(storage)->length = newLength;
        ((ReserveWordBits*)storage)->capacity = newCapacity;
    } else {
        InsertTemp tmp;
        unsigned int empty = 0;
        if (tail != 0 && (unsigned int)(oldData + position) + insertLength < (unsigned int)last &&
            (unsigned int)last <= (unsigned int)(oldData + length)) {
            unsigned int tmpCount = tmp.data == 0 ? (((unsigned char*)&tmp)[0] & kInlineLengthMask) : 0;
            volatile unsigned int spare;
            unsigned char allocByte = *(volatile unsigned char*)&spare;
            StdString_ReplaceRange(&tmp, 0, tmpCount, first, last, &allocByte);
            if (tmp.word >> 31)
                first = tmp.data;
            else
                first = (char*)&tmp + 1;
        }
        if (tail != 0)
            memmove((void*)((unsigned int)(oldData + position) + insertLength),
                    (void*)((unsigned int)(oldData + position) + removed), tail);
        memmove(oldData + position, first, insertLength);
        oldData[newLength] = s_stdStringRangeInPlaceTerminator;
        if (!usesHeap)
            ((ReserveByteBits*)storage)->length = newLength;
        else
            AsMWStringStorage(storage)->length = newLength;
    }
    return self;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Replace a range with repeated bytes using the standard-string allocator.
void* StdString_ReplaceRepeatedChar(void* self, unsigned int position, unsigned int eraseCount,
                                    unsigned int repeatCount, signed char fillCharacter) {
    unsigned char* storage = (unsigned char*)self;
    unsigned int word = *(unsigned int*)storage;
    int usesHeap = word >> 31;
    char* oldData = usesHeap ? AsMWStringStorage(storage)->data : (char*)storage + 1;
    unsigned int capacity = usesHeap ? word & 0x7fffffff : kInlineCapacity;
    unsigned int length = usesHeap ? AsMWStringStorage(storage)->length : storage[0] & kInlineLengthMask;
    if (position > length) {
        throw std::out_of_range(stringErrorStrings_AF820.outOfRange);
    }
    unsigned int suffix = length - position;
    unsigned int removedLength = eraseCount < suffix ? eraseCount : suffix;
    unsigned int limit = 0x7ffffffe;
    if (repeatCount > limit || length - removedLength > limit - repeatCount) {
        throw std::length_error(stringErrorStrings_AF820.lengthError);
    }
    unsigned int newLen = length - removedLength + repeatCount;
    if (newLen >= capacity) {
        unsigned int newCap = (capacity + 15) & ~15;
        while (newCap < newLen + 1)
            newCap = (newCap * 2 + 15) & ~15;
        char* data = (char*)__nw__FUl(newCap);
        if (position)
            memcpy(data, oldData, position);
        memset(data + position, (unsigned char)fillCharacter, repeatCount);
        if (suffix - removedLength)
            memcpy(data + position + repeatCount, oldData + position + removedLength, suffix - removedLength);
        data[newLen] = s_stdStringRepeatGrowTerminator;
        if (usesHeap)
            __dl__FPv(oldData);
        else
            *(unsigned int*)storage |= 0x80000000;
        AsMWStringStorage(storage)->data = data;
        AsMWStringStorage(storage)->length = newLen;
        *(unsigned int*)storage = (*(unsigned int*)storage & 0x80000000) | newCap;
    } else {
        if (suffix - removedLength)
            memmove(oldData + position + repeatCount, oldData + position + removedLength, suffix - removedLength);
        memset(oldData + position, (unsigned char)fillCharacter, repeatCount);
        oldData[newLen] = s_stdStringRepeatInPlaceTerminator;
        if (!usesHeap)
            storage[0] = (storage[0] & 0x80) | (newLen & kInlineLengthMask);
        else
            AsMWStringStorage(storage)->length = newLen;
    }
    return self;
}
#pragma pop

// Restore an absolute stream position after validating mode, bounds, and append restrictions.
void MWStringBuf_SeekPosition(void* output, void* state, void* input, unsigned int mode) {
    unsigned int* out = (unsigned int*)output;
    unsigned char* v = (unsigned char*)state;
    if ((mode & kInputOutputMode) == 0) {
        out[0] = out[1] = 0xFFFFFFFF;
        out[2] = 0;
        return;
    }
    unsigned int reading = mode & kInputMode;
    if ((reading && ((AsMWStreamBuffer(v)->mode & kInputMode) == 0)) ||
        ((mode &= kOutputMode) && ((AsMWStreamBuffer(v)->mode & kOutputMode) == 0))) {
        out[0] = out[1] = 0xFFFFFFFF;
        out[2] = 0;
        return;
    }
    if ((reading && AsMWStreamBuffer(v)->getCurrent == 0) || (mode && AsMWStreamBuffer(v)->putCurrent == 0)) {
        out[0] = out[1] = 0xFFFFFFFF;
        out[2] = 0;
        return;
    }
    if ((*(char**)&AsMWStreamBuffer(v)->highWater) < (*(char**)&AsMWStreamBuffer(v)->putCurrent))
        (*(char**)&AsMWStreamBuffer(v)->highWater) = (*(char**)&AsMWStreamBuffer(v)->putCurrent);
    if ((*(char**)&AsMWStreamBuffer(v)->getEnd) &&
        (*(char**)&AsMWStreamBuffer(v)->getEnd) < (*(char**)&AsMWStreamBuffer(v)->highWater))
        (*(char**)&AsMWStreamBuffer(v)->getEnd) = (*(char**)&AsMWStreamBuffer(v)->highWater);
    long long position = *(long long*)input;
    int difference;
    if (mode)
        difference = (*(char**)&AsMWStreamBuffer(v)->highWater) - (*(char**)&AsMWStreamBuffer(v)->putBegin);
    else
        difference = (*(char**)&AsMWStreamBuffer(v)->highWater) - (*(char**)&AsMWStreamBuffer(v)->getBegin);
    long long amount = difference;
    if (position < 0 || position > amount) {
        out[0] = out[1] = 0xFFFFFFFF;
        out[2] = 0;
        return;
    }
    if ((AsMWStreamBuffer(v)->mode & kAppendMode) && mode && position != amount) {
        out[2] = 0;
        out[0] = out[1] = 0xFFFFFFFF;
        return;
    }
    if (reading)
        (*(char**)&AsMWStreamBuffer(v)->getCurrent) = (*(char**)&AsMWStreamBuffer(v)->getBegin) + (int)position;
    if (mode)
        (*(char**)&AsMWStreamBuffer(v)->putCurrent) = (*(char**)&AsMWStreamBuffer(v)->putBegin) + (int)position;
    unsigned int hi = ((unsigned int*)input)[0];
    unsigned int lo = ((unsigned int*)input)[1];
    unsigned int extra = ((unsigned int*)input)[2];
    out[1] = lo;
    out[0] = hi;
    out[2] = extra;
}
// Default putback hook reports failure.
int MWStreamBuffer_DefaultPutbackFailure() {
    return -1;
}
// Default overflow hook reports failure.
int MWStreamBuffer_DefaultOverflow() {
    return -1;
}
// Copy the string from the stream buffer embedded at offset 0x14.
int MWStringStream_CopyBufferString(void* first, char* second) {
    return MWStringBuf_GetString(first, second + 0x14);
}
// Read up to the requested byte count, refilling through the virtual hook as needed.
int MWStreamBuffer_Read(void* first, void* destination, int length) {
    void* self = first;
    void* output = destination;
    int requested = length;
    int available = (*(int*)&AsMWStreamBuffer(self)->getEnd) - (*(int*)&AsMWStreamBuffer(self)->getCurrent);
    int count;
    if (available < requested) {
        count = available;
    } else {
        count = requested;
    }
    if (count > 0) {
        memcpy(output, (*(void**)&AsMWStreamBuffer(self)->getCurrent), count);
        output = (char*)output + count;
        (*(void**)&AsMWStreamBuffer(self)->getCurrent) = (char*)(*(void**)&AsMWStreamBuffer(self)->getCurrent) + count;
        requested -= count;
    }
    while (requested > 0) {
        int byte;
        void* current = (*(void**)&AsMWStreamBuffer(self)->getCurrent);
        if ((unsigned int)current < AsMWStreamBuffer(self)->getEnd) {
            (*(void**)&AsMWStreamBuffer(self)->getCurrent) = (char*)current + 1;
            byte = *(unsigned char*)current;
        } else {
            DispatchFn read = ((DispatchFn*)*(void**)self)[11];
            int result = ((int (*)(void*))read)(self);
            if (result == -1)
                break;
            byte = result;
        }
        *(unsigned char*)output = byte;
        output = (char*)output + 1;
        ++count;
        --requested;
    }
    return count;
}
// Select the smaller requested or available buffer size by reference.
static inline const int& MinBufferTransfer(const int& a, const int& b) {
    return (b < a) ? b : a;
}
class C38 {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual int v38(int);
};
// Write up to the requested byte count, invoking overflow when the put area is full.
int MWStreamBuffer_Write(void* first, void* source, int length) {
    void* self = first;
    void* input = source;
    int available;
    int requested = length;
    available = (*(int*)&AsMWStreamBuffer(self)->putEnd) - (*(int*)&AsMWStreamBuffer(self)->putCurrent);
    int count = MinBufferTransfer(requested, available);
    if (count > 0) {
        memcpy((*(void**)&AsMWStreamBuffer(self)->putCurrent), input, count);
        input = (char*)input + count;
        (*(int*)&AsMWStreamBuffer(self)->putCurrent) += count;
        requested -= count;
    }
    while (requested > 0) {
        void* current = (*(void**)&AsMWStreamBuffer(self)->putCurrent);
        signed char byte = *(unsigned char*)input;
        input = (char*)input + 1;
        int result;
        if ((unsigned int)current < AsMWStreamBuffer(self)->putEnd) {
            *(unsigned char*)current = byte;
            (*(void**)&AsMWStreamBuffer(self)->putCurrent) = (char*)current + 1;
            result = *(unsigned char*)current;
        } else {
            result = ((C38*)self)->v38((unsigned char)byte);
        }
        if (result == -1)
            break;
        ++count;
        --requested;
    }
    return count;
}
// Copy the active get or put range into a new game string.
// first is uninitialized result-string storage; second is a borrowed stream buffer.
// The result owns its copied bytes. This reconstructed int signature has no explicit
// return statement: callers must use the constructed object, not a return code.
int MWStringBuf_GetString(void* first, void* second) {
    void* self = first;
    unsigned char* value = (unsigned char*)second;
    if ((AsMWStreamBuffer(value)->mode & kOutputMode) != 0) {
        unsigned int limit = AsMWStreamBuffer(value)->putCurrent;
        unsigned int end = AsMWStreamBuffer(value)->highWater;
        if (end < limit) {
            end = limit;
            AsMWStreamBuffer(value)->highWater = limit;
        }
        unsigned int start = AsMWStreamBuffer(value)->putBegin;
        *(int*)self = 0;
        (*(int*)&AsMWStringStorage(self)->length) = 0;
        (*(int*)&AsMWStringStorage(self)->data) = 0;
        String_ReserveCapacity(self, end - start);
        volatile unsigned char local[5];
        local[4] = local[0];
        String_ReplaceRange(self, 0, 0, (char*)start, (char*)end, (unsigned char*)local + 4);
    } else {
        unsigned int end = AsMWStreamBuffer(value)->getEnd;
        unsigned int start = AsMWStreamBuffer(value)->getBegin;
        *(int*)self = 0;
        (*(int*)&AsMWStringStorage(self)->length) = 0;
        (*(int*)&AsMWStringStorage(self)->data) = 0;
        String_ReserveCapacity(self, end - start);
        volatile unsigned char local[5];
        local[4] = local[0];
        String_ReplaceRange(self, 0, 0, (char*)start, (char*)end, (unsigned char*)local + 4);
    }
    // The original returns no value: its callers never read one.
}
// Report that this message-table context supports the operation.
int CSerifuEcb_vfunc_4() {
    return 1;
}
// Report that this message-table context is ready.
int CSerifuEcb_vfunc_3() {
    return 1;
}
// Report that this message-table context is available.
int CSerifuEcb_vfunc_2() {
    return 1;
}
// Report successful message-table context status.
int CSerifuEcb_vfunc_1() {
    return 1;
}
// Return the data pointer from the context's attached resource.
void* MessageContext_GetResourceData(void* value) {
    return ((ResourceDataView*)((MessageResourceView*)value)->table)->data;
}
// Install the primary table, grouped resources, and text-table format.
int MessageContext_SetTablesAndFormat(void* value, void* primaryTable, void* groupedTables, void* tableFormat) {
    AsCSerifuEcb(value)->table = primaryTable;
    AsCSerifuEcb(value)->groupedTextTables = groupedTables;
    (*(void**)&AsCSerifuEcb(value)->formatType) = tableFormat;
    return 1;
}
// Return the locale decimal-point character with signed-char promotion.
int MWNumPunct_GetDecimalPoint(void* value) {
    unsigned char byte = (*(unsigned char*)&((NumPunctView*)value)->decimalPoint);
    return (signed char)byte;
}
// Dispatch the locale facet's grouping-string accessor.
void MWNumPunct_GetGrouping(void* first, void* second) {
    (((DispatchFn*)*(void**)second)[5])(first, second);
}
// Return the locale thousands-separator character with signed-char promotion.
int MWNumPunct_GetThousandsSeparator(void* value) {
    return (signed char)(*(unsigned char*)&((NumPunctView*)value)->thousandsSeparator);
}
// Copy the punctuation facet's grouping string, retaining inline storage when possible.
void StdString_CopyFromOffset12(void* output, void* source) {
    unsigned int flags = ((NumPunctView*)source)->grouping.flags;
    if ((flags >> 31) == 0) {
        unsigned int second = ((NumPunctView*)source)->grouping.length;
        *(unsigned int*)output = flags;
        AsMWStringStorage(output)->length = second;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = (*(unsigned int*)&((NumPunctView*)source)->grouping.data);
    } else {
        *(unsigned int*)output = 0;
        AsMWStringStorage(output)->length = 0;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = 0;
        volatile unsigned int tmp;
        StdString_ReserveCapacity(output, ((NumPunctView*)source)->grouping.length);
        unsigned char local = *((volatile unsigned char*)&tmp);
        unsigned int start = (*(unsigned int*)&((NumPunctView*)source)->grouping.data);
        unsigned int length = ((NumPunctView*)source)->grouping.length;
        StdString_ReplaceRange(output, 0, 0, (char*)start, (char*)(start + length), &local);
    }
}
#pragma push
#pragma use_lmw_stmw on
// Copy a Metrowerks string into independent inline or heap storage.
void StdString_CopyConstructStorage(void* output, void* unused, const void* source) {
    unsigned int flags = *(const unsigned int*)source;
    if ((flags >> 31) == 0) {
        *(unsigned int*)output = flags;
        AsMWStringStorage(output)->length = *(const unsigned int*)((const char*)source + 4);
        (*(unsigned int*)&AsMWStringStorage(output)->data) = *(const unsigned int*)((const char*)source + 8);
    } else {
        *(unsigned int*)output = 0;
        AsMWStringStorage(output)->length = 0;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = 0;
        StdString_ReserveCapacity(output, *(const unsigned int*)((const char*)source + 4));
        volatile unsigned int tmp;
        unsigned char local = *((volatile unsigned char*)&tmp);
        unsigned int start = *(const unsigned int*)((const char*)source + 8);
        unsigned int length = *(const unsigned int*)((const char*)source + 4);
        StdString_ReplaceRange(output, 0, 0, (char*)start, (char*)(start + length), &local);
    }
}
#pragma pop

// Release grouping and boolean-name strings owned by a punctuation facet.
void* MWNumPunct_Destroy(char* value, int deleteFlag) {
    if (value != 0) {
        if (value + 0x24 != 0 && (*(unsigned int*)(value + 0x24) >> 31) != 0)
            __dl__FPv(*(void**)(value + 0x2c));
        if (value + 0x18 != 0 && (*(unsigned int*)(value + 0x18) >> 31) != 0)
            __dl__FPv(*(void**)(value + 0x20));
        if (value + 0x0c != 0 && (*(unsigned int*)(value + 0x0c) >> 31) != 0)
            __dl__FPv(*(void**)(value + 0x14));
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return (void*)value;
}
struct VDisp64D8 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void call(int*, void*, signed char);
};
// Forward a copied output iterator to the numeric facet's formatting slot.
// Slot 0x24 is MWStream_FormatDouble(double, void*, CheckedNumericOutput*, void*, char); the original keeps the
// value in f1.
void MWNumPut_ForwardIteratorCopy(void* self, int* value, void* arg3, signed char arg4) {
    int local = *value;
    ((VDisp64D8*)self)->call(&local, arg3, arg4);
}
// Dispatch the punctuation facet's localized true-name accessor.
int StdNumPunct_GetTrueName(void* first, void* second) {
    void* table = *(void**)second;
    DispatchFn function = ((DispatchFn*)table)[6];
    return function(first, second);
}
// Dispatch the punctuation facet's localized false-name accessor.
int StdNumPunct_GetFalseName(void* first, void* second) {
    void* table = *(void**)second;
    DispatchFn function = ((DispatchFn*)table)[7];
    return function(first, second);
}
struct Sb6D9C {
    void** vt;
    char pad[0x10];
    unsigned char* pptr;
    unsigned char* epptr;
};
// A single stream-buffer pointer is an output iterator; null records sticky write failure.
struct It6D9C {
    Sb6D9C* sb;
};
typedef It6D9C HexOutputIterator;
typedef Sb6D9C HexStreamBuffer;
// Retain the shared locale until the enclosing formatting operation leaves scope.
struct Loc6D9C {
    void** arr;
    void* ref;
    // Retain the stream locale so fetched facet pointers remain valid in scope.
    Loc6D9C(void* s) {
        ((void (*)(void*, void*))MWStream_CopyLocale)(this, s);
    }
    // Release the retained locale on normal or exception exit.
    ~Loc6D9C() throw() {
        if (ref)
            MWSharedOwner_ReleaseReference(ref);
    }
};
typedef Loc6D9C RetainedHexLocale;

// Lazily allocate the character-classification facet identifier.
static inline unsigned int GetCtypeFacetId() {
    if (!s_ctypeFacetId)
        s_ctypeFacetId = ++s_localeFacetIdCounter;
    return s_ctypeFacetId;
}
struct Guard6D9C {
    unsigned char c;
    // Initialize the local facet-construction marker.
    Guard6D9C() : c(0) {}
    // No owned allocation is attached to this marker.
    ~Guard6D9C() {}
};
// Find or create the character-classification facet in a retained locale.
static inline void* GetHexCtypeFacet(const Loc6D9C& loc) {
    Guard6D9C guard;
    unsigned int idx = GetCtypeFacetId();
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        f = __nw__FUl(0x18);
        if (f)
            ((void (*)(void*, int, int, bool))MWCtypeFacet_Construct)(f, 0, 0, false);
        void** a = loc.arr;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(a, f, GetCtypeFacetId());
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
// Write one byte and mark the iterator failed if overflow fails.
static inline void WriteHexIteratorByte(HexOutputIterator& it, char c) {
    bool failed = false;
    if (it.sb) {
        HexStreamBuffer* sb = it.sb;
        int r;
        unsigned char* p = sb->pptr;
        if (p < sb->epptr) {
            *p = c;
            sb->pptr = p + 1;
            r = *p;
        } else {
            r = ((int (*)(HexStreamBuffer*, int))sb->vt[0x38 / 4])(sb, (unsigned char)c);
        }
        if (r == -1)
            failed = true;
    }
    if (failed)
        it.sb = 0;
}
// Write the requested number of padding characters.
static inline HexOutputIterator PadHexIterator(HexOutputIterator it, int n, char c) {
    int i = 0;
    if (n > 0)
        for (; i < n; i++)
            WriteHexIteratorByte(it, c);
    return it;
}
// Copy a character range through the output iterator.
static inline HexOutputIterator WriteHexIteratorText(HexOutputIterator it, const char* p, int n) {
    for (int i = 0; i < n; i++)
        WriteHexIteratorByte(it, *p++);
    return it;
}
// Replace only the selected format-flag group.
static inline void ReplaceStreamFlagGroup(unsigned char* s, unsigned short fl, unsigned short mask) {
    AsMWStreamState(s)->flags = (AsMWStreamState(s)->flags & ~mask) | (fl & mask);
}
// Enable additional format flags without clearing other groups.
static inline void EnableStreamFlags(unsigned char* s, unsigned short fl) {
    AsMWStreamState(s)->flags |= fl;
}
// Save formatting for a nested numeric operation and restore it on normal or exception exit.
struct Saver6D9C {
    unsigned char* s;
    unsigned int width;
    unsigned int prec;
    unsigned short flags;
    unsigned char except;
    // Snapshot flags, width, precision and exception mask before nested formatting.
    Saver6D9C(unsigned char* st)
        : s(st), width(AsMWStreamState(st)->width), prec(AsMWStreamState(st)->precision),
          flags(AsMWStreamState(st)->flags), except(((StreamIosView*)st)->exceptions) {}
    // Restore saved formatting and enforce the restored stream exception mask.
    ~Saver6D9C() {
        unsigned char* st = s;
        AsMWStreamState(st)->flags = flags;
        AsMWStreamState(st)->width = width;
        AsMWStreamState(st)->precision = prec;
        ((StreamIosView*)st)->exceptions = except;
        if (((StreamIosView*)st)->buffer == 0)
            ((StreamIosView*)st)->state |= 1;
        if (((StreamIosView*)st)->exceptions & ((StreamIosView*)st)->state)
            MWStream_ThrowFailure(st, streamRestoreFailureMessage);
    }
};
#pragma push
#pragma use_lmw_stmw on
typedef Saver6D9C SavedHexFormatting;

// Format a 32-bit value as ten characters with 0x prefix and internal zero padding.
void* Stream_WriteFixedWidthHex32(void* self, HexOutputIterator* out, void* source, char fill, unsigned int value) {
    unsigned char* streamBytes = (unsigned char*)source;
    char fieldText[12];
    char digits[0x24];
    // Phase 1: format exactly ten bytes under temporary hexadecimal flags.
    {
        SavedHexFormatting saver(streamBytes);
        ReplaceStreamFlagGroup(streamBytes, kHexadecimal, kBaseField);
        EnableStreamFlags(streamBytes, kShowBase);
        ReplaceStreamFlagGroup(streamBytes, kInternal, kAdjustField);
        AsMWStreamState(streamBytes)->width = 10;
        void* zeroCharacterFacet = GetHexCtypeFacet(RetainedHexLocale(streamBytes));
        char zeroFillCharacter =
            ((char (*)(void*, char))(*(void***)zeroCharacterFacet)[0x1c / 4])(zeroCharacterFacet, '0');
        int prefixLength = 0;
        void* prefixCharacterFacet = GetHexCtypeFacet(RetainedHexLocale(streamBytes));
        char pre[4];
        if (AsMWStreamState(streamBytes)->flags & kShowBase) {
            pre[0] = ((char (*)(void*, char))(*(void***)prefixCharacterFacet)[0x1c / 4])(prefixCharacterFacet, '0');
            char* q = pre + 1;
            if (AsMWStreamState(streamBytes)->flags & kUppercase)
                *q = ((char (*)(void*, char))(*(void***)prefixCharacterFacet)[0x1c / 4])(prefixCharacterFacet, 'X');
            else
                *q = ((char (*)(void*, char))(*(void***)prefixCharacterFacet)[0x1c / 4])(prefixCharacterFacet, 'x');
            prefixLength += 2;
        }
        int digitCount = ((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(
            streamBytes, value, digits, prefixCharacterFacet, 0);
        char* d = fieldText;
        int zeroPaddingCount = 0;
        if ((*(int*)&AsMWStreamState(streamBytes)->width) > prefixLength + digitCount)
            zeroPaddingCount = (*(int*)&AsMWStreamState(streamBytes)->width) - (prefixLength + digitCount);
        unsigned short numberAlignment = AsMWStreamState(streamBytes)->flags & kAdjustField;
        if (numberAlignment != kLeft && numberAlignment != kInternal)
            for (int i = 0; i < zeroPaddingCount; i++)
                *d++ = zeroFillCharacter;
        {
            char* p = pre;
            for (int i = 0; i < prefixLength; i++)
                *d++ = *p++;
        }
        if (numberAlignment == kInternal)
            for (int i = 0; i < zeroPaddingCount; i++)
                *d++ = zeroFillCharacter;
        {
            char* p = digits;
            for (int i = 0; i < digitCount; i++)
                *d++ = *p++;
        }
        if (numberAlignment == kLeft)
            for (int i = 0; i < zeroPaddingCount; i++)
                *d++ = zeroFillCharacter;
        AsMWStreamState(streamBytes)->width = 0;
    }
    // Phase 2: emit the completed field through the sticky-failure output iterator.
    HexOutputIterator it = *out;
    int outerPaddingCount = 0;
    if ((*(int*)&AsMWStreamState(streamBytes)->width) > 10)
        outerPaddingCount = (*(int*)&AsMWStreamState(streamBytes)->width) - 10;
    unsigned short fieldAlignment = AsMWStreamState(streamBytes)->flags & kAdjustField;
    if (fieldAlignment != kLeft && fieldAlignment != kInternal)
        it = PadHexIterator(it, outerPaddingCount, fill);
    HexOutputIterator afterInternalPadding = it;
    if (fieldAlignment == kInternal)
        afterInternalPadding = PadHexIterator(it, outerPaddingCount, fill);
    HexOutputIterator afterFieldText = WriteHexIteratorText(afterInternalPadding, fieldText, 10);
    HexOutputIterator afterTrailingPadding = afterFieldText;
    if (fieldAlignment == kLeft)
        afterTrailingPadding = PadHexIterator(afterFieldText, outerPaddingCount, fill);
    AsMWStreamState(streamBytes)->width = 0;
    return afterTrailingPadding.sb;
}
#pragma pop
// Store the caller's current context in the shared global.
void SharedContext_SetCurrent(void* value) {
    g_currentSharedContext = value;
}
// Optionally free the message-resource allocation.
void* CMsgEcbResource_dtor(void* value, int deleteFlag) {
    if (value != 0 && deleteFlag > 0) {
        __dl__FPv(value);
    }
    return value;
}
// Optionally free this trivial string-support allocation.
void* CMsgFormatCtrl_dtor(void* value, int deleteFlag) {
    if (value != 0 && deleteFlag > 0) {
        __dl__FPv(value);
    }
    return value;
}
// Optionally free this second trivial string-support allocation.
void* CSerifuEcb_dtor(void* value, int deleteFlag) {
    if (value != 0 && deleteFlag > 0) {
        __dl__FPv(value);
    }
    return value;
}
// Release a retained locale reference and optionally free the locale handle.
void* LocaleRef_dtor(void* value, short deleteFlag) {
    if (value != 0) {
        void* child = ((StreamLocaleView*)value)->reference;
        if (child != 0) {
            try {
                MWSharedOwner_ReleaseReference(child);
            } catch (...) {
                __unexpected();
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}
// Release the stream's embedded locale reference and optionally free the stream.
void* MWInputStream_Destroy(void* value, short deleteFlag) {
    if (value != 0) {
        if ((char*)value + (int)&((InputStreamLocaleView*)0)->locale != 0) {
            void* child = ((InputStreamLocaleView*)value)->locale.reference;
            if (child != 0) {
                try {
                    MWSharedOwner_ReleaseReference(child);
                } catch (...) {
                    __unexpected();
                }
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}
// Optionally free this string-support allocation.
void* dtor_800B1144(void* value, int deleteFlag) {
    if (value != 0 && deleteFlag > 0) {
        __dl__FPv(value);
    }
    return value;
}
// Optionally free this formatting-support allocation.
void* MWFormatSupport_DeleteTrivialObject(void* value, int deleteFlag) {
    if (value != 0 && deleteFlag > 0) {
        __dl__FPv(value);
    }
    return value;
}
extern void Text_AppendMappedFormattedValue(void*, void*);
struct TagEntry {
    int tagKind;
    unsigned short* name;
};
struct TagHandler {
    unsigned short* name;
    int (*handler)(void*);
};
struct TagText {
    unsigned int word;
    unsigned int length;
    char* data;
    // Construct an empty owning byte string for a narrowed tag key.
    TagText() : word(0), length(0), data(0) {}
    // Release heap storage owned by the narrowed key.
    ~TagText() {
        if (word >> 31)
            appFreeUni__FPv(data);
    }
};
struct TagGroup {
    void* head;
    char* vtable;
    // Leave the borrowed message-table data intact.
    ~TagGroup() {}
};
// Match a UTF-16 tag name before its closing delimiter and return the following character.
static inline unsigned short* find_tag(unsigned short* text, unsigned short* limit, unsigned short* name) {
    unsigned short* start = text;
    if (!start)
        return 0;
    while (*start) {
        unsigned short* hit = 0;
        if (start) {
            unsigned short* p = start;
            unsigned short* q = start;
            for (int i = 0; *q; ++p, ++q, ++i) {
                if (limit == p)
                    break;
                if (name[0] == *q) {
                    hit = start + i;
                    break;
                }
            }
        }
        if (!hit)
            return 0;
        unsigned short* a = hit + 1;
        unsigned short* b = name + 1;
        int differs = 0;
        while (*b) {
            if (*a != *b) {
                differs = 1;
                break;
            }
            ++a;
            ++b;
        }
        if (!differs)
            return a;
        start = hit + 1;
    }
    return 0;
}
// Append one UTF-16 code unit to a small or heap-backed string.
static inline void append_wide(WStringObject* text, unsigned short c) {
    unsigned int length;
    // The byte load is the inline length tag; word 1 is WStringObject::heapLength.
    if ((*(unsigned int*)text >> 31) == 0)
        length = *(unsigned char*)text & kInlineLengthMask;
    else
        length = ((unsigned int*)text)[1];
    volatile unsigned int tmp;
    unsigned char local = *((volatile unsigned char*)&tmp);
    WString_ReplaceRange(text, length, 0, &c, &c + 1, &local);
}
#pragma push
#pragma use_lmw_stmw on
// Expand formatting and named-text tags into UTF-16 output, skipping carriage returns.
// Grammar over NUL-terminated UTF-16: ordinary code units copy literally (CR is
// dropped); backslash followed by <...> starts a table-recognized tag. The kind
// table contains V= (mapped value), L= (discard), D= (named text). V= handlers
// recognize ITEM_FLOWER and PRIORITY; D= uses the first space-delimited name.
// Matching searches for the table spelling within the tag, rather than requiring
// an anchored prefix. Unknown tags copy literally. Missing D= replacements leave
// scanning at the introducer; the next iteration copies from < onwards literally.
// output borrows an initialized 12-byte WString object (storage flag/inline text
// at +0, heap length at +4, heap pointer at +8), never a plain UTF-16 buffer.
// append_wide appends to existing content and may allocate. source
// and messageTable are borrowed, must not alias output storage across growth.
// Recognized tags require a closing >; D= names must fit 255 code units plus NUL.
// There is no malformed-tag/length validation. Replacement text copies literally
// (no recursion); allowNamedLookup selects the hash/name path only for format 1.
// Examples (backslash is a literal input code unit): A\<L=ignored>B -> AB;
// A\<D= GREETING>B -> AHelloB if GREETING resolves to Hello; plain A/CR/B -> AB.
// The D= key is narrowed by keeping each code unit's low byte, not by UTF-8 encoding.
// A wide key U+0141 therefore searches for byte 0x41. Replacement text stays UTF-16.
// Returns 1 on normal completion; allocation failures may throw.
int Text_ExpandTaggedText(void* messageTable, WStringObject* output, unsigned short* source, void* allowNamedLookup) {
    unsigned short wideLookupKey[kTagLookupCodeUnits];
    for (; *source; ++source) {
        unsigned int ch = *source;
        if (ch == kCarriageReturn)
            continue;
        if (ch != '\\') {
            append_wide(output, ch);
            continue;
        }
        if (source[1] != '<') {
            append_wide(output, ch);
            continue;
        }
        // Phase 1: locate closing delimiter, then resolve the tag kind in table order.
        unsigned short* tagBegin = source + 2;
        unsigned short* tagEnd = 0;
        if (tagBegin) {
            unsigned short* p = tagBegin;
            unsigned short* q = tagBegin;
            for (int i = 0; *q; ++p, ++q, ++i) {
                if (!p)
                    break;
                if (*q == '>') {
                    tagEnd = tagBegin + i;
                    break;
                }
            }
        }
        unsigned short* tagArgument = 0;
        int tagKind = kUnknownMessageTag;
        {
            TagEntry* entry = (TagEntry*)s_messageTagKindTable;
            for (int i = 0; i < kTagKindCount; ++i, ++entry) {
                tagArgument = find_tag(tagBegin, tagEnd, entry->name);
                if (tagArgument) {
                    tagKind = entry->tagKind;
                    break;
                }
            }
        }
        switch (tagKind) {
        case kMappedValueTag: {
            TagHandler* entry = (TagHandler*)s_messageValueTagHandlers;
            int result = -1;
            for (int i = 0; i < kMappedTagHandlerCount; ++i, ++entry) {
                if (find_tag(tagArgument, tagEnd, entry->name)) {
                    result = entry->handler(tagArgument);
                    break;
                }
            }
            Text_AppendMappedFormattedValue(output, (void*)result);
            source = tagEnd;
            break;
        }
        case kDiscardedTag:
            source = tagEnd;
            break;
        case kNamedTextTag: {
            // Phase 2: narrow the name to bytes for lookup, then append resolved UTF-16.
            unsigned short* leadingSpaces = tagArgument;
            int spaceCount = 0;
            while (*leadingSpaces) {
                if (*leadingSpaces != ' ')
                    break;
                ++leadingSpaces;
                ++spaceCount;
            }
            unsigned short* name = tagArgument + spaceCount;
            int count = 0;
            unsigned short* keySource = name;
            unsigned short* keyBoundaryProbe = name;
            unsigned short* keyDestination = wideLookupKey;
            while (*keySource) {
                if (*keySource == ' ' || tagEnd == keyBoundaryProbe)
                    break;
                *keyDestination = *keySource;
                ++keySource;
                ++keyBoundaryProbe;
                ++keyDestination;
                ++count;
            }
            // Caller contract: count < kTagLookupCodeUnits. Preserve the original
            // unchecked write; this code does not truncate or validate tag names.
            wideLookupKey[count] = 0;
            unsigned short* replacement;
            {
                TagText narrowLookupKey;
                volatile unsigned int tmp;
                unsigned char alloc = *((volatile unsigned char*)&tmp);
                for (unsigned short* p = wideLookupKey; *p; ++p) {
                    char keyByte = (char)*p;
                    String_ReplaceRange(&narrowLookupKey, fn_string_len(&narrowLookupKey), 0, &keyByte, &keyByte + 1,
                                        &alloc);
                }
                // Phase 3: select the format-1 table or grouped named-entry lookup.
                // Hash equality is only a prefilter; exact byte-name equality is required.
                if (AsCSerifuEcb(messageTable)->formatType == kNamedTextTable && allowNamedLookup) {
                    char* key = (char*)fn_string_ptr(&narrowLookupKey);
                    int hash = Hash_XorStringBytes(key);
                    TagGroup group;
                    group.head = 0;
                    group.head = (AsCSerifuEcb(messageTable)->table) ? AsCSerifuEcb(messageTable)->table : group.head;
                    group.vtable = __vt__10CSerifuEcb;
                    int entryIndex = -1;
                    void* entry;
                    for (;;) {
                        int size = group.head ? ((MessageTableHeader*)group.head)->entryCount : 0;
                        if (size - 1 <= entryIndex) {
                            entry = 0;
                            break;
                        }
                        ++entryIndex;
                        if (entryIndex < 0)
                            entryIndex = 0;
                        entry = Table_GetEntry(group.head, (void*)entryIndex);
                        if (!entry)
                            break;
                        void* node = *(void**)entry;
                        int matched = *(int*)Table_ResolveDataOffset(group.head, node) == hash;
                        if (matched) {
                            void* keyNode = Table_ResolveDataOffset(group.head, node);
                            char* storedKey = keyNode ? (char*)OffsetStringNode_GetText(keyNode) : 0;
                            matched = strcmp(key, storedKey) == 0;
                        }
                        if (matched) {
                            entry = Table_ResolveDataOffset(group.head, ((MessageTableEntry*)entry)->namedTextOffset);
                            break;
                        }
                    }
                    replacement = (unsigned short*)entry;
                } else {
                    replacement =
                        (unsigned short*)TextResource_FindNamedEntry(messageTable, fn_string_ptr(&narrowLookupKey));
                }
            }
            // Phase 4: append replacement text literally; a miss leaves the tag unconsumed.
            if (!replacement)
                continue;
            for (; *replacement; ++replacement)
                append_wide(output, *replacement);
            source = tagEnd;
            break;
        }
        default:
            append_wide(output, ch);
            break;
        }
    }
    return 1;
}
#pragma pop
// Resolve an indexed message and expand its tags into the destination.
void MessageTable_ExpandEntryText(void* messageTable, WStringObject* output, void* index, void* allowNamedLookup) {
    int value = MessageTable_GetTextByIndex(messageTable, index);
    Text_ExpandTaggedText(messageTable, output, (unsigned short*)value, allowNamedLookup);
}
// Extend the readable range to the high-water mark and peek the current byte.
int MWStringBuf_Underflow(char* value) {
    if ((*(void**)&AsMWStreamBuffer(value)->getCurrent) == 0)
        return -1;
    unsigned char* end = (*(unsigned char**)&AsMWStreamBuffer(value)->putCurrent);
    unsigned char* current = (*(unsigned char**)&AsMWStreamBuffer(value)->highWater);
    if (current < end)
        (*(unsigned char**)&AsMWStreamBuffer(value)->highWater) = end;
    unsigned char* reloaded = (*(unsigned char**)&AsMWStreamBuffer(value)->highWater);
    unsigned char* position = (*(unsigned char**)&AsMWStreamBuffer(value)->getEnd);
    if (position < reloaded)
        (*(unsigned char**)&AsMWStreamBuffer(value)->getEnd) = reloaded;
    position = (*(unsigned char**)&AsMWStreamBuffer(value)->getEnd);
    unsigned char* buffer = (*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent);
    (void)end;
    (void)current;
    if (buffer < position)
        return buffer[0];
    return -1;
}
// Replace a game string's contents with a NUL-terminated byte string.
int NString_AssignCString(void* value, char* string) {
    unsigned int flags = *(unsigned int*)value;
    unsigned int length;
    if ((flags >> 31) == 0)
        length = *(unsigned char*)value & kInlineLengthMask;
    else
        length = AsMWStringStorage(value)->length;
    volatile unsigned int tmp;
    unsigned char local = *((volatile unsigned char*)&tmp);
    unsigned int stringLength = strlen(string);
    String_ReplaceRange(value, 0, length, string, string + stringLength, &local);
    return 1;
}
// Construct a standard string from a NUL-terminated byte string.
void StdString_ConstructCString(void* value, char* string) {
    *(int*)value = 0;
    (*(int*)&AsMWStringStorage(value)->length) = 0;
    (*(int*)&AsMWStringStorage(value)->data) = 0;
    unsigned int length = strlen(string);
    StdString_ReserveCapacity(value, length);
    volatile unsigned int tmp;
    unsigned char local = *((volatile unsigned char*)&tmp);
    StdString_ReplaceRange(value, 0, 0, string, string + length, &local);
}
#pragma push
#pragma use_lmw_stmw on
// Search grouped text tables by hash and exact key, resolving the text offset on success.
int TextResource_FindNamedEntry(void* resource, void* name) {
    void* container = AsCSerifuEcb(resource)->groupedTextTables;
    int wanted = Hash_XorStringBytes(name);
    int groupIndex = 0;
    while (groupIndex < ((OffsetTableHeader*)container)->groupCount) {
        void* group = OffsetTable_GetEntry(container, groupIndex);
        int entryIndex = 0;
        while (entryIndex < ((MessageTableHeader*)group)->entryCount) {
            void* entry = Table_GetEntry(group, (void*)entryIndex);
            void* node = Table_ResolveDataOffset(group, *(void**)entry);
            if (wanted == *(int*)node) {
                void* key = OffsetStringNode_GetText(node);
                if (strcmp((const char*)name, (const char*)key) == 0)
                    return (int)Table_ResolveDataOffset(group, *((void**)entry + 1));
            }
            ++entryIndex;
        }
        ++groupIndex;
    }
    return 0;
}
#pragma pop
// Resolve an indexed entry using the active indexed or named table format.
int MessageTable_GetTextByIndex(void* messageTable, void* index) {
    int type = AsCSerifuEcb(messageTable)->formatType;
    if (type != kNamedTextTable) {
        if (type != kIndexedTextTable)
            return 0;
        unsigned char local[8];
        *(void**)local = 0;
        void* ptr = AsCSerifuEcb(messageTable)->table;
        if (ptr != 0)
            *(void**)local = ptr;
        *(void**)(local + 4) = __vt__14CMsgFormatCtrl;
        void* item = Table_GetEntry(*(void**)local, index);
        return (int)Table_ResolveDataOffset(*(void**)local, ((MessageTableEntry*)item)->textOffset);
    }
    unsigned char local[8];
    *(void**)local = 0;
    void* ptr = AsCSerifuEcb(messageTable)->table;
    if (ptr != 0)
        *(void**)local = ptr;
    *(void**)(local + 4) = __vt__10CSerifuEcb;
    void* item = Table_GetEntry(*(void**)local, index);
    return (int)Table_ResolveDataOffset(*(void**)local, ((MessageTableEntry*)item)->namedTextOffset);
}
// Restore saved stream formatting and exception policy, then apply the restored exception mask.
void* StreamFormattingSnapshot_dtor(void* value, int deleteFlag) {
    if (value != 0) {
        AsMWStreamState(*(void**)value)->flags = ((StreamFormattingSnapshot*)value)->flags;
        (*(void**)&AsMWStreamState(*(void**)value)->width) = (*(void**)&((StreamFormattingSnapshot*)value)->width);
        (*(void**)&AsMWStreamState(*(void**)value)->precision) =
            (*(void**)&((StreamFormattingSnapshot*)value)->precision);
        void* object = *(void**)value;
        ((StreamIosView*)object)->exceptions = ((StreamFormattingSnapshot*)value)->exceptions;
        if (((StreamIosView*)object)->buffer == 0)
            ((StreamIosView*)object)->state |= kStreamBad;
        unsigned char tmp32 = ((StreamIosView*)object)->state;
        unsigned char tmp33 = ((StreamIosView*)object)->exceptions;
        if (tmp33 & tmp32)
            MWStream_ThrowFailure(object, streamRestoreFailureMessage);
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}
// Copy the punctuation facet's localized true-name string.
void MWNumPunct_CopyTrueName(void* output, void* source) {
    unsigned int flags = ((NumPunctView*)source)->trueName.flags;
    if ((flags >> 31) == 0) {
        unsigned int second = ((NumPunctView*)source)->trueName.length;
        *(unsigned int*)output = flags;
        AsMWStringStorage(output)->length = second;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = (*(unsigned int*)&((NumPunctView*)source)->trueName.data);
    } else {
        *(unsigned int*)output = 0;
        AsMWStringStorage(output)->length = 0;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = 0;
        volatile unsigned int tmp;
        unsigned int length = ((NumPunctView*)source)->trueName.length;
        StdString_ReserveCapacity(output, length);
        unsigned char local = *((volatile unsigned char*)&tmp);
        unsigned int start = (*(unsigned int*)&((NumPunctView*)source)->trueName.data);
        unsigned int size = ((NumPunctView*)source)->trueName.length;
        StdString_ReplaceRange(output, 0, 0, (char*)start, (char*)(start + size), &local);
    }
}
// Copy the punctuation facet's localized false-name string.
void MWNumPunct_CopyFalseName(void* output, void* source) {
    unsigned int flags = ((NumPunctView*)source)->falseName.flags;
    if ((flags >> 31) == 0) {
        unsigned int second = ((NumPunctView*)source)->falseName.length;
        *(unsigned int*)output = flags;
        AsMWStringStorage(output)->length = second;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = (*(unsigned int*)&((NumPunctView*)source)->falseName.data);
    } else {
        *(unsigned int*)output = 0;
        AsMWStringStorage(output)->length = 0;
        (*(unsigned int*)&AsMWStringStorage(output)->data) = 0;
        volatile unsigned int tmp;
        unsigned int length = ((NumPunctView*)source)->falseName.length;
        StdString_ReserveCapacity(output, length);
        unsigned char local = *((volatile unsigned char*)&tmp);
        unsigned int start = (*(unsigned int*)&((NumPunctView*)source)->falseName.data);
        unsigned int size = ((NumPunctView*)source)->falseName.length;
        StdString_ReplaceRange(output, 0, 0, (char*)start, (char*)(start + size), &local);
    }
}
// Release a heap-backed standard string and optionally free its object.
void* MWString_dtor(void* value, int deleteFlag) {
    if (value != 0) {
        unsigned int flags = *(unsigned int*)value;
        if ((flags >> 31) != 0)
            __dl__FPv((*(void**)&AsMWStringStorage(value)->data));
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}
// Release a second standard-string wrapper and optionally free its object.
void* MWStringStorage_Release(void* value, int deleteFlag) {
    if (value != 0) {
        if (value != 0) {
            unsigned int flags = *(unsigned int*)value;
            if ((flags >> 31) != 0)
                __dl__FPv((*(void**)&AsMWStringStorage(value)->data));
        }
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}
// Move back one byte, allowing replacement only for writable or unchanged input.
int MWStringBuf_PutbackFailure(void* value, int byte) {
    if ((*(unsigned char**)&AsMWStreamBuffer(value)->getBegin) >=
        (*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent))
        return -1;
    unsigned char* end = (*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent);
    if (byte == -1) {
        --end;
        (*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent) = end;
        return (byte == -1) ? 0 : byte;
    }
    unsigned char flags = AsMWStreamBuffer(value)->mode;
    signed char signedByte = (signed char)byte;
    if ((flags & kOutputMode) == 0 && signedByte != (signed char)end[-1])
        return -1;
    (*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent) =
        (*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent) - 1;
    *((*(unsigned char**)&AsMWStreamBuffer(value)->getCurrent)) = (unsigned char)signedByte;
    return byte;
}
// Flush unit-buffered output after a successful operation unless already flushing.
void* OStreamSentry_dtor(void* value, int deleteFlag) {
    if (value != 0) {
        void* reference = ((StreamSentryView*)value)->reference;
        void* object = *(void**)reference;
        unsigned char flags = ((StreamIosView*)object)->state;
        if ((flags & kStreamBadOrFail) == 0 && (AsMWStreamState(object)->flags & kUnitBuffer) != 0 &&
            ((StreamSentryView*)value)->flushing == 0)
            OStream_Flush(reference);
        if (deleteFlag > 0)
            __dl__FPv(value);
    }
    return value;
}

typedef int (*Widen5778Fn)(void*, signed char);
// Convert an ASCII byte through the locale's character-widening slot.
static inline int WidenUnsignedOutputDigit(void* facet, signed char c) {
    return (((Widen5778Fn*)*(void**)facet)[7])(facet, c);
}
// Retain a locale long enough to obtain its character-classification facet.
static inline void* GetUnsignedOutputCtypeFacet(void* ios) {
    Loc6D9C loc(ios);
    Guard6D9C guard;
    unsigned int idx = GetCtypeFacetId();
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        f = __nw__FUl(0x18);
        if (f)
            ((void (*)(void*, int, int, bool))MWCtypeFacet_Construct)(f, 0, 0, false);
        void** a = loc.arr;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(a, f, GetCtypeFacetId());
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
// Write repeated padding bytes and preserve output-iterator failure.
static inline void PadUnsignedOutput(void** pout, int count, signed char c) {
    int i = 0;
    if (count > 0)
        for (; i < count; ++i) {
            int failed = 0;
            if (*pout != 0) {
                if (bput_byte(pout, c) == -1)
                    failed = 1;
            }
            if (failed)
                *pout = 0;
        }
}
// Write a byte range and preserve output-iterator failure.
static inline void WriteUnsignedOutputText(void** pout, unsigned char* data, int count) {
    int i = 0;
    if (count > 0)
        for (; i < count; ++i) {
            signed char c = (signed char)*data;
            ++data;
            int failed = 0;
            if (*pout != 0) {
                if (bput_byte(pout, c) == -1)
                    failed = 1;
            }
            if (failed)
                *pout = 0;
        }
}
#pragma push
#pragma use_lmw_stmw on
// Write an unsigned 64-bit value with base prefixes, locale grouping, and width padding.
// iter contains a borrowed stream-buffer pointer; returned null denotes failed output.
// Prefix, internal padding, digits, trailing left-padding are emitted in that order
// after any right-alignment padding. Width resets to zero for the completed field.
void* Stream_WriteUnsigned64(void* unused, void** iter, void* ios, unsigned char fill, unsigned long long number) {
    unsigned int numHi = (unsigned int)(number >> 32);
    unsigned int numLo = (unsigned int)number;
    unsigned char* s = (unsigned char*)ios;
    // Phase 1: choose octal/hex/decimal and build the corresponding prefix/digits.
    // Each branch then applies the same width/adjustment policy without sharing
    // volatile iterator snapshots, preserving each call site's original stores.
    switch (AsMWStreamState(s)->flags & kBaseField) {
    case kOctal: {
        void* volatile out = (void*)*(void**)iter;
        unsigned char buffer[0x60];
        void* writer = GetUnsignedOutputCtypeFacet(ios);
        void* volatile spare1;
        void* volatile spare2;
        void* volatile spare3;
        void* volatile spare4;
        void* volatile spare5;
        int count = 0;
        unsigned char* cursor = buffer;
        if (((numLo ^ 0) | (numHi ^ 0)) == 0 || (AsMWStreamState(s)->flags & kShowBase) != 0) {
            *cursor = (unsigned char)WidenUnsignedOutputDigit(writer, '0');
            ++cursor;
            ++count;
        }
        if (((numLo ^ 0) | (numHi ^ 0)) != 0)
            count += MWNumPut_ConvertUnsigned64Digits(ios, number, (char*)cursor, writer, 0);
        // Phase 2: emit the prepared field and retain failure across every write.
        int width = (*(int*)&AsMWStreamState(s)->width);
        int remaining = 0;
        void* volatile canon = out;
        if (width > count)
            remaining = width - count;
        unsigned short adj = AsMWStreamState(s)->flags & kAdjustField;
        if (adj != kLeft && adj != kInternal) {
            void* volatile pad = out;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* keep = (void*)pad;
            spare1 = keep;
            canon = keep;
        }
        void* keep2 = (void*)canon;
        spare2 = keep2;
        void* volatile cur = keep2;
        if (adj == kInternal) {
            void* volatile pad = cur;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* keep3 = (void*)pad;
            spare3 = keep3;
            cur = keep3;
        }
        void* volatile txt = cur;
        WriteUnsignedOutputText((void**)&txt, buffer, count);
        void* done = (void*)txt;
        spare4 = done;
        void* volatile result = done;
        if (adj == kLeft) {
            void* volatile pad = done;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* fin = (void*)pad;
            spare5 = fin;
            result = fin;
        }
        AsMWStreamState(s)->width = 0;
        return (void*)result;
    }
    case kHexadecimal: {
        void* volatile out = (void*)*(void**)iter;
        int prefixCount = 0;
        unsigned char prefix[0x10];
        unsigned char digits[0x20];
        void* writer = GetUnsignedOutputCtypeFacet(ios);
        void* volatile spare1;
        void* volatile spare2;
        void* volatile spare3;
        void* volatile spare4;
        void* volatile spare5;
        if ((AsMWStreamState(s)->flags & kShowBase) != 0) {
            prefix[0] = (unsigned char)WidenUnsignedOutputDigit(writer, '0');
            if ((AsMWStreamState(s)->flags & kUppercase) != 0)
                prefix[1] = (unsigned char)WidenUnsignedOutputDigit(writer, 'X');
            else
                prefix[1] = (unsigned char)WidenUnsignedOutputDigit(writer, 'x');
            prefixCount = 2;
        }
        int digitCount = MWNumPut_ConvertUnsigned64Digits(ios, number, (char*)digits, writer, 0);
        int width = (*(int*)&AsMWStreamState(s)->width);
        int total = prefixCount + digitCount;
        void* volatile canon = out;
        int remaining = 0;
        if (total < width)
            remaining = width - total;
        unsigned short adj = AsMWStreamState(s)->flags & kAdjustField;
        if (adj != kLeft && adj != kInternal) {
            void* volatile pad = out;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* keep = (void*)pad;
            spare1 = keep;
            canon = keep;
        }
        void* volatile txt = canon;
        WriteUnsignedOutputText((void**)&txt, prefix, prefixCount);
        void* keep2 = (void*)txt;
        spare2 = keep2;
        void* volatile cur = keep2;
        if (adj == kInternal) {
            void* volatile pad = keep2;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* keep3 = (void*)pad;
            spare3 = keep3;
            cur = keep3;
        }
        void* volatile txt2 = cur;
        WriteUnsignedOutputText((void**)&txt2, digits, digitCount);
        void* done = (void*)txt2;
        spare4 = done;
        void* volatile result = done;
        if (adj == kLeft) {
            void* volatile pad = done;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* fin = (void*)pad;
            spare5 = fin;
            result = fin;
        }
        AsMWStreamState(s)->width = 0;
        return (void*)result;
    }
    default: {
        void* volatile out = (void*)*(void**)iter;
        unsigned char digits[0x28];
        void* writer = GetUnsignedOutputCtypeFacet(ios);
        void* volatile spare1;
        void* volatile spare2;
        void* volatile spare3;
        void* volatile spare4;
        void* volatile spare5;
        int digitCount = MWNumPut_ConvertUnsigned64Digits(ios, number, (char*)digits, writer, 0);
        int width = (*(int*)&AsMWStreamState(s)->width);
        int remaining = 0;
        void* volatile canon = out;
        if (width > digitCount)
            remaining = width - digitCount;
        unsigned short adj = AsMWStreamState(s)->flags & kAdjustField;
        if (adj != kLeft && adj != kInternal) {
            void* volatile pad = out;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* keep = (void*)pad;
            spare1 = keep;
            canon = keep;
        }
        void* keep2 = (void*)canon;
        spare2 = keep2;
        void* volatile cur = keep2;
        if (adj == kInternal) {
            void* volatile pad = cur;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* keep3 = (void*)pad;
            spare3 = keep3;
            cur = keep3;
        }
        void* volatile txt = cur;
        WriteUnsignedOutputText((void**)&txt, digits, digitCount);
        void* done = (void*)txt;
        spare4 = done;
        void* volatile result = done;
        if (adj == kLeft) {
            void* volatile pad = done;
            PadUnsignedOutput((void**)&pad, remaining, fill);
            void* fin = (void*)pad;
            spare5 = fin;
            result = fin;
        }
        AsMWStreamState(s)->width = 0;
        return (void*)result;
    }
    }
    (void)unused;
}
#pragma pop

// Formatter wrapper contracts (the address-suffixed ABI type names are retained):
// String9C owns a 12-byte byte-string; data() borrows bytes until mutation/destruction.
// Facet9C owns grouping/true/false strings; the locale owns an installed facet.
// Writer9C borrows ctype and widens one byte through slot 5. FacetDispatch9C borrows
// numpunct and reads its separator through slot 2; the interfaces are not interchangeable.
// Loc6D9C retains the locale; fetched facets borrow that lifetime. Guard6D9C is
// a construction marker with no allocation. Raw8E54 only requests uninitialized storage.
struct Raw8E54 {};
// Standard-string ABI wrapper; raw construction is reserved for routines that fill all words.
struct String9C {
    unsigned int words[3];
    // Initialize an empty owning standard-string representation.
    String9C() {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
    }
    // Reserve uninitialized result storage; callee must fill every string word.
    String9C(Raw8E54*) {}
    // Copy a borrowed NUL-terminated byte string into owned storage.
    String9C(char* text) {
        ((void (*)(void*, char*, signed char))StdString_ConstructCString)(this, text, 0);
    }
    // Copy the borrowed punctuation facet's grouping string.
    String9C(void* facet) {
        ((void* (*)(void*, void*))MWNumPunct_GetGrouping)(this, facet);
    }
    // Free owned heap bytes, leaving inline storage in the wrapper.
    ~String9C() {
        if (words[0] >> 31)
            __dl__FPv((void*)words[2]);
    }
    // Test whether characters live inline in the wrapper.
    bool small() const {
        return !(words[0] >> 31);
    }
    // Return the active representation's length in bytes.
    unsigned int size() const {
        return small() ? (*(unsigned char*)this & kInlineLengthMask) : words[1];
    }
    // Borrow mutable characters until the next string mutation.
    char* data() const {
        return small() ? (char*)this + 1 : (char*)words[2];
    }
    // Copy inline words directly; replace the range when either string owns heap storage.
    void assign(const String9C& other) {
        bool heap = words[0] >> 31;
        unsigned int first;
        if (!heap && !((first = other.words[0]) >> 31)) {
            unsigned int a = first, b = other.words[1];
            words[0] = a;
            words[1] = b;
            words[2] = other.words[2];
        } else {
            unsigned int n = heap ? words[1] : (*(unsigned char*)this & kInlineLengthMask);
            char* p;
            unsigned int len;
            if (!(other.words[0] >> 31)) {
                p = (char*)&other + 1;
                len = *(unsigned char*)&other & kInlineLengthMask;
            } else {
                p = (char*)other.words[2];
                len = other.words[1];
            }
            volatile unsigned int allocator;
            struct Allocator {
                unsigned char byte;
                unsigned char padding[3];
            } local;
            local.byte = *(volatile unsigned char*)&allocator;
            StdString_ReplaceRange(this, 0, n, p, p + len, (unsigned char*)&local);
        }
    }
};
struct FacetBase9C {
    void* vt;
    unsigned int refs;
    // Initialize an unreferenced punctuation facet base.
    FacetBase9C() {
        refs = 0;
        vt = numPunctCharVtable;
    }
    // No base-owned allocation needs cleanup in this local view.
    ~FacetBase9C() {}
};
struct Facet9C : FacetBase9C {
    unsigned char point, separator;
    unsigned short pad;
    String9C grouping, trueName, falseName;
    // Initialize punctuation with dot/comma and owned default true/false names.
    Facet9C() : point('.'), separator(',') {
        trueName.assign(String9C(boolTrueNameString));
        falseName.assign(String9C(falseKeywordString));
    }
};
struct Writer9C {
    virtual void slot0() = 0;
    virtual void slot1() = 0;
    virtual void slot2() = 0;
    virtual void slot3() = 0;
    virtual void slot4() = 0;
    virtual char widen(signed char) = 0;
};
struct FacetDispatch9C {
    virtual void slot0() = 0;
    virtual void slot1() = 0;
    virtual unsigned int separator() = 0;
};
typedef String9C OwnedFacetString;
typedef Facet9C DefaultNumericPunctuation;
typedef Writer9C CharacterWidenInterface;
typedef FacetDispatch9C PunctuationSeparatorInterface;
typedef Loc6D9C RetainedNumericLocale;
typedef Guard6D9C FacetConstructionMarker;

// Reverse the generated digits and separators into display order.
static inline void ReverseFormattedDigits(char* first, char* last) {
    if (first != last) {
        while (first < --last) {
            char saved = *first;
            *first++ = *last;
            *last = saved;
        }
    }
}
#pragma push
#pragma use_lmw_stmw on
// Convert an unsigned 64-bit value to locale digits in the selected base, with optional grouping.
// output is caller-provided writable space for digits plus locale separators;
// returned byte count excludes a terminator (none is written). writer/source are
// borrowed facets/stream state. number is the unsigned 64-bit magnitude.
int MWNumPut_ConvertUnsigned64Digits(void* source, unsigned long long number, char* output, void* writer,
                                     int suppressGrouping) {
    RetainedNumericLocale locale(source);
    unsigned char* localeBytes = (unsigned char*)&locale;
    if (!number) {
        *output = ((CharacterWidenInterface*)writer)->widen((signed char)'0');
        return 1;
    }
    FacetConstructionMarker facetConstructionMarker;
    if (s_numPunctCharFacetId == 0)
        s_numPunctCharFacetId = ++s_localeFacetIdCounter;
    void* punctuationFacet;
    int index = s_numPunctCharFacetId;
    if (index >= ((unsigned int*)locale.arr)[1] || (punctuationFacet = ((void**)locale.arr[0])[index]) == 0) {
        punctuationFacet = new DefaultNumericPunctuation;
        void* array = *(void**)localeBytes;
        if (!s_numPunctCharFacetId)
            s_numPunctCharFacetId = ++s_localeFacetIdCounter;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(array, punctuationFacet, s_numPunctCharFacetId);
        punctuationFacet = ((void**)locale.arr[0])[index];
    }
    // Phase 1: resolve punctuation/grouping and select the numeric base.
    unsigned int base = 10;

    switch ((AsMWStreamState(source)->flags) & kBaseField) {
    case kOctal:
        base = 8;
        break;
    case kHexadecimal:
        base = 16;
        break;
    }
    char* nextDigit = output;
    OwnedFacetString grouping(punctuationFacet);
    unsigned int groupingLength = grouping.size();
    bool useGrouping = groupingLength != 0;
    unsigned int groupIndex = 0;
    unsigned char digitsInGroup = 0;
    unsigned char groupSize = 0;
    if (useGrouping) {
        groupSize = grouping.data()[0];
        if (groupSize == 0)
            useGrouping = 0;
    }
    if (suppressGrouping)
        useGrouping = 0;
    unsigned int groupSeparator;
    if (useGrouping) {
        groupSeparator = ((PunctuationSeparatorInterface*)punctuationFacet)->separator();
    } else
        groupSeparator = 0;
    bool useUpper = (AsMWStreamState(source)->flags & kUppercase) != 0;
    // Phase 2: generate least-significant digits first; grouping consumes digit
    // counts from right to left, reuses the final group size, and zero disables it.
    while (number != 0) {
        unsigned int baseValue = base;
        char digit;
        unsigned long long rem = __mod2u(number >> 32, (unsigned int)number, 0, baseValue);
        if (10 > rem)
            digit = ((CharacterWidenInterface*)writer)->widen((signed char)(rem + 0x30));
        else if (useUpper)
            digit = ((CharacterWidenInterface*)writer)->widen((signed char)(rem + 0x37));
        else
            digit = ((CharacterWidenInterface*)writer)->widen((signed char)(rem + 0x57));
        *nextDigit = digit;
        nextDigit = nextDigit + 1;
        unsigned int baseValue2 = base;
        number = __div2u(number >> 32, (unsigned int)number, 0, baseValue2);
        if (0 != number) {
            if (useGrouping) {
                ++digitsInGroup;
                if ((unsigned int)digitsInGroup == (unsigned int)groupSize) {
                    *nextDigit = (char)groupSeparator;
                    nextDigit = nextDigit + 1;
                    unsigned int len = grouping.size();
                    groupIndex = groupIndex + 1;
                    if (groupIndex < len) {
                        unsigned int groupingIndex = groupIndex;
                        groupSize = grouping.data()[groupingIndex];
                        if (!groupSize)
                            useGrouping = 0;
                    }
                    digitsInGroup = 0;
                }
            }
        }
    }
    // Phase 3: reverse the complete byte range into display order (no NUL appended).
    ReverseFormattedDigits(output, nextDigit);
    return (int)(nextDigit - output);
}
// Lazily allocate the numeric-punctuation facet identifier.
static inline unsigned int GetIntegerPunctuationFacetId() {
    if (!s_numPunctCharFacetId)
        s_numPunctCharFacetId = ++s_localeFacetIdCounter;
    return s_numPunctCharFacetId;
}
// Convert an unsigned 32-bit value to locale digits with optional grouping.
// Same caller-owned unterminated digit-buffer contract as the 64-bit converter;
// suppressGrouping is treated as a boolean value, never dereferenced.
int MWNumPut_ConvertUnsigned32Digits(void* source, unsigned int number, char* output, void* writer,
                                     void* suppressGrouping, int /*unused*/) {
    RetainedNumericLocale locale(source);
    unsigned char* localeBytes = (unsigned char*)&locale;
    if (number == 0) {
        *output = ((CharacterWidenInterface*)writer)->widen((signed char)'0');
        return 1;
    }
    FacetConstructionMarker facetConstructionMarker;
    if (!s_numPunctCharFacetId)
        s_numPunctCharFacetId = ++s_localeFacetIdCounter;
    void* punctuationFacet;
    unsigned int index = s_numPunctCharFacetId;
    if (index >= ((unsigned int*)locale.arr)[1] || (punctuationFacet = ((void**)locale.arr[0])[index]) == 0) {
        punctuationFacet = new DefaultNumericPunctuation;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(locale.arr, punctuationFacet,
                                                                      GetIntegerPunctuationFacetId());
        punctuationFacet = ((void**)locale.arr[0])[index];
    }
    // Phase 1: resolve punctuation/grouping and select the numeric base.
    unsigned int base = 10;
    unsigned short srcFlags = AsMWStreamState(source)->flags;
    switch (srcFlags & kBaseField) {
    case kOctal:
        base = 8;
        break;
    case kHexadecimal:
        base = 16;
        break;
    }
    char* nextDigit = output;
    OwnedFacetString grouping(punctuationFacet);
    unsigned int groupingLength = grouping.size();
    bool useGrouping = groupingLength != 0;
    unsigned int groupIndex = 0;
    unsigned char digitsInGroup = 0;
    unsigned char groupSize = 0;
    if (useGrouping) {
        groupSize = grouping.data()[0];
        if (groupSize == 0)
            useGrouping = 0;
    }
    if (suppressGrouping)
        useGrouping = 0;
    unsigned int groupSeparator;
    if (useGrouping) {
        groupSeparator = ((PunctuationSeparatorInterface*)punctuationFacet)->separator();
    } else
        groupSeparator = 0;
    unsigned int upperBit = AsMWStreamState(source)->flags & kUppercase;
    bool useUpper = upperBit != 0;
    // Phase 2: generate least-significant digits first; grouping consumes digit
    // counts from right to left, reuses the final group size, and zero disables it.
    while (number != 0) {
        unsigned int rem = number % base;
        char digit;
        if (rem < 10)
            digit = ((CharacterWidenInterface*)writer)->widen((signed char)(rem + 0x30));
        else if (useUpper)
            digit = ((CharacterWidenInterface*)writer)->widen((signed char)(rem + 0x37));
        else
            digit = ((CharacterWidenInterface*)writer)->widen((signed char)(rem + 0x57));
        *nextDigit = digit;
        ++nextDigit;
        number /= base;
        if (number != 0 && useGrouping) {
            ++digitsInGroup;
            if ((unsigned int)digitsInGroup == (unsigned int)groupSize) {
                unsigned int separatorByte = groupSeparator;
                *nextDigit = (char)separatorByte;
                ++nextDigit;
                unsigned int len = grouping.size();
                ++groupIndex;
                if (groupIndex < len) {
                    groupSize = grouping.data()[groupIndex];
                    if (!groupSize)
                        useGrouping = 0;
                }
                digitsInGroup = 0;
            }
        }
    }
    {
        char saved;
        char* last = nextDigit;
        char* first = output;
        if (first != last) {
            while (first < --last) {
                saved = *first;
                *first++ = *last;
                *last = saved;
            }
        }
    }
    return (int)(nextDigit - output);
}

struct Facet6514 {
    char storage[0x18];
    // Initialize the default byte-character classification facet.
    Facet6514() {
        ((void (*)(void*, void*, bool, unsigned int))MWCtypeFacet_Construct)(this, 0, false, 0);
    }
};
// Find or create a character-classification facet for numeric output.
static inline void* GetNumericCtypeFacet(const Loc6D9C& loc) {
    Guard6D9C guard;
    unsigned int idx = GetCtypeFacetId();
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        Facet6514* fresh = new Facet6514;
        void** array = loc.arr;
        unsigned int id = GetCtypeFacetId();
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(array, fresh, id);
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
struct Upper6514 {
    virtual void slot0() = 0;
    virtual void slot1() = 0;
    virtual void upper(char*, char*) = 0;
};
// Checked output contract: the one-pointer iterator borrows its stream buffer.
// sputc writes at pptr then advances, or calls overflow at vtable slot 12.
// EOF (-1) clears the iterator pointer; subsequent byte/padding/range writes do
// nothing. Iterator copies propagate this failure value and never own the buffer.
struct Sb6514 {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3() = 0;
    virtual void v4() = 0;
    virtual void v5() = 0;
    virtual void v6() = 0;
    virtual void v7() = 0;
    virtual void v8() = 0;
    virtual void v9() = 0;
    virtual void v10() = 0;
    virtual void v11() = 0;
    virtual int overflow(int) = 0;
    char pad[16];
    unsigned char* pptr;
    unsigned char* epptr;
    // Write one byte in place or call virtual overflow when the put area is full.
    int sputc(char c) {
        unsigned char* p = pptr;
        if (p < epptr) {
            *p = c;
            pptr = p + 1;
            return *p;
        }
        return overflow((unsigned char)c);
    }
};
struct It6514 {
    Sb6514* sb;
};
struct Source6514 {
    char pad[0x2c];
    int width;
    unsigned short flags;
};
typedef Sb6514 NumericOutputBuffer;
typedef It6514 CheckedNumericOutput;
typedef Source6514 NumericFormattingState;
// Write one byte, clearing the iterator buffer on output failure.
static inline void WriteNumericIteratorByte(CheckedNumericOutput& it, char c) {
    bool failed = false;
    if (it.sb && it.sb->sputc(c) == -1)
        failed = true;
    if (failed)
        it.sb = 0;
}
// Emit fill characters through the checked output iterator.
static inline CheckedNumericOutput PadNumericIterator(CheckedNumericOutput it, int n, char c) {
    int i = 0;
    if (n > 0)
        for (; i < n; ++i)
            WriteNumericIteratorByte(it, c);
    return it;
}
// Emit a character range through the checked output iterator.
static inline CheckedNumericOutput WriteNumericIteratorText(CheckedNumericOutput it, const char* p, int n) {
    int i = 0;
    if (n > 0)
        for (; i < n; ++i)
            WriteNumericIteratorByte(it, *p++);
    return it;
}
// Emit sign/base prefix and digits with left, right, or internal padding, then clear width.
static inline CheckedNumericOutput WriteNumericParts(CheckedNumericOutput it, int signCount, char* sign, int length,
                                                     char* data, NumericFormattingState* s, char fill) {
    int padding = 0;
    int width = s->width;
    if (width > signCount + length)
        padding = width - (signCount + length);
    unsigned short adjust = s->flags & kAdjustField;
    if (adjust != kLeft && adjust != kInternal)
        it = PadNumericIterator(it, padding, fill);
    CheckedNumericOutput result;
    result = WriteNumericIteratorText(it, sign, signCount);
    if (adjust == kInternal)
        result = PadNumericIterator(result, padding, fill);
    result = WriteNumericIteratorText(result, data, length);
    if (adjust == kLeft)
        result = PadNumericIterator(result, padding, fill);
    s->width = 0;
    return result;
}
// Format sign and magnitude, including NaN/infinity, with the selected float notation.
CheckedNumericOutput MWStream_FormatDouble(double value, void* unused, CheckedNumericOutput* out, void* source,
                                           char fill) {
    NumericFormattingState* s = (NumericFormattingState*)source;
    void* entry = GetNumericCtypeFacet(Loc6D9C(source));
    int signCount = 0;
    char sign;
    bool negative = ((int (*)(double))__signbitd)(value) != 0;
    if (!negative && (s->flags & kShowPositive)) {
        signCount = 1;
        sign = ((CharacterWidenInterface*)entry)->widen((signed char)'+');
    } else if (negative) {
        signCount = 1;
        sign = ((CharacterWidenInterface*)entry)->widen((signed char)'-');
        value = -value;
    }
    OwnedFacetString parts;
    if (__fpclassifyd(value) > 2) {
        switch (s->flags & kFloatField) {
        case kFixed:
            MWStream_FormatDoubleFixed(source, &parts, value);
            break;
        case kScientific:
            ((void (*)(void*, void*, double))MWStream_FormatDoubleScientific)(source, &parts, value);
            break;
        default:
            ((void (*)(void*, void*, double))MWStream_FormatDoubleGeneral)(source, &parts, value);
            break;
        }
    } else {
        if (__fpclassifyd(value) == 1)
            parts.assign(OwnedFacetString(s_mwNaNText));
        else
            parts.assign(OwnedFacetString(s_mwInfinityText));
        if (s->flags & kUppercase) {
            void* upper = GetNumericCtypeFacet(Loc6D9C(source));
            unsigned int n = parts.size();
            char* end = parts.data() + n;
            ((Upper6514*)upper)->upper(parts.data(), end);
        }
    }
    int length = parts.size();
    char* data = parts.data();
    return WriteNumericParts(*out, signCount, &sign, length, data, s, fill);
}

// General formatting's ctype interface has five ABI slots preceding widen,
// matching CharacterWidenInterface; unused slot roles remain unresolved.
struct GeneralFormatCharacterFacet {
    virtual void s2() = 0;
    virtual void s3() = 0;
    virtual void s4() = 0;
    virtual void s5() = 0;
    virtual void s6() = 0;
    virtual char widen(signed char) = 0;
};
// Punctuation ABI: one unused slot precedes decimal point and grouping separator.
// This view borrows a locale facet; it never destroys or owns that facet.
struct GeneralFormatPunctuationFacet {
    virtual void s2() = 0; // ABI padding; never called by this view.
    virtual char point() = 0;
    virtual char separator() = 0;
};
// Find or create the locale's numeric-punctuation facet.
static inline void* GetGeneralFormatPunctuation(const Loc6D9C& loc) {
    Guard6D9C guard;
    if (!s_numPunctCharFacetId)
        s_numPunctCharFacetId = ++s_localeFacetIdCounter;
    unsigned int idx = s_numPunctCharFacetId;
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        Facet9C* fresh = new Facet9C;
        void** array = loc.arr;
        if (!s_numPunctCharFacetId)
            s_numPunctCharFacetId = ++s_localeFacetIdCounter;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(array, fresh, s_numPunctCharFacetId);
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
// Find the first requested byte in an inline or heap-backed string.
static inline int FindFormatCharacter(OwnedFacetString& s, signed char c) {
    unsigned int n = s.size();
    if (n != 0) {
        char* base = s.data();
        for (char* p = base; p < base + n; ++p)
            if (c == *p)
                return p - base;
    }
    return -1;
}
// Erase a byte-string range, retaining the recovered volatile allocator-token copy.
// StdString_ReplaceRange does not dereference allocPlace; it only forwards it on
// overlapping insertion. This call inserts [null,null), so that path is not used.
// The copied byte is indeterminate ABI residue, not an initialized allocator ID.
// Its read is preserved for code identity; this comment does not make it defined C++.
static inline void EraseFormatRange(OwnedFacetString& s, unsigned int pos, unsigned int n) {
    volatile unsigned int allocator;
    struct Allocator {
        unsigned char byte;
        unsigned char padding[3];
    } local;
    local.byte = *(volatile unsigned char*)&allocator;
    StdString_ReplaceRange(&s, pos, n, 0, 0, (unsigned char*)&local);
}
#pragma push
#pragma use_lmw_stmw on
// Choose fixed/scientific general formatting and trim trailing zeros unless showpoint is set.
void MWStream_FormatDoubleGeneral(void* stream, void* output, double value) {
    unsigned char* s = (unsigned char*)stream;
    OwnedFacetString& str = *(OwnedFacetString*)output;
    Saver6D9C saver(s);
    int precision = saver.prec;
    if (precision == 0)
        precision = 1;
    AsMWStreamState(s)->precision = precision - 1;
    // Phase 1: build scientific digits at precision-1 and obtain their decimal exponent.
    int exponent = ((int (*)(void*, void*, double))MWStream_FormatDoubleScientific)(stream, output, value);
    Loc6D9C locale(stream);
    void* ctype = GetNumericCtypeFacet(locale);
    void* punct = GetGeneralFormatPunctuation(locale);
    signed char zero = ((GeneralFormatCharacterFacet*)ctype)->widen('0');
    signed char point = ((GeneralFormatPunctuationFacet*)punct)->point();
    // Phase 2: exponent in [-4,precision) chooses fixed notation; otherwise keep
    // scientific notation. Trimming and locale grouping follow that choice.
    if (exponent >= -4 && exponent < precision) {
        if (str.data()[1] == point)
            EraseFormatRange(str, precision + 1, -1);
        else
            EraseFormatRange(str, 1, -1);
        if (exponent < 0) {
            if (str.data()[1] == point)
                EraseFormatRange(str, 1, 1);
            StdString_ReplaceRepeatedChar(&str, 0, 0, -exponent, zero);
            StdString_ReplaceRepeatedChar(&str, 1, 0, 1, point);
        } else if (exponent > 0) {
            EraseFormatRange(str, 1, 1);
            if (exponent < precision - 1 || (AsMWStreamState(s)->flags & kShowPoint))
                StdString_ReplaceRepeatedChar(&str, exponent + 1, 0, 1, point);
        }
        if (!(AsMWStreamState(s)->flags & kShowPoint)) {
            if (FindFormatCharacter(str, point) != -1) {
                char* end = str.data() + str.size();
                char* p = end - 1;
                while (zero == *p)
                    --p;
                ++p;
                if (p != end)
                    EraseFormatRange(str, p - str.data(), end - p);
                if (str.data()[str.size() - 1] == point) {
                    unsigned char* head = (unsigned char*)&str;
                    if (str.small()) {
                        unsigned int n = (*head & kInlineLengthMask) - 1;
                        head[1 + n] = 0;
                        *head = (*head & 0x80) | (n & kInlineLengthMask);
                    } else {
                        unsigned int n = str.words[1] - 1;
                        ((char*)str.words[2])[n] = 0;
                        str.words[1] = n;
                    }
                }
            }
        }
        OwnedFacetString grouping(punct);
        char separator = ((GeneralFormatPunctuationFacet*)punct)->separator();
        NumericText_InsertGroupingSeparators((signed char)point, (signed char)separator, &grouping, &str);
    } else {
        unsigned short flags = AsMWStreamState(s)->flags;
        if (!(flags & kShowPoint) && str.data()[1] == point) {
            signed char e;
            if (flags & kUppercase)
                e = ((GeneralFormatCharacterFacet*)ctype)->widen('E');
            else
                e = ((GeneralFormatCharacterFacet*)ctype)->widen('e');
            char* epos = str.data() + FindFormatCharacter(str, e);
            char* p = epos - 1;
            while (zero == *p)
                --p;
            ++p;
            if (p != epos)
                EraseFormatRange(str, p - str.data(), epos - p);
            if (str.data()[2] == e)
                EraseFormatRange(str, 1, 1);
        }
    }
}
#pragma pop

extern int MWDecimalDigits_ShouldRoundUp(void*, int);
extern void MWDecimalDigits_RoundSignificant(void*, void*, int, int*);
struct Dec8E54 {
    OwnedFacetString digits;
    short pad;
    short exp;
    // Reserve raw digit-result storage for the decimal conversion routine.
    Dec8E54() : digits((Raw8E54*)0) {}
};
struct SmallStr8E54 {
    CW_BITS2(unsigned char heap : 1, unsigned char len : 7) char data[1];
};
// Replace a string with one character and a trailing NUL.
static inline void AssignSingleFormatCharacter(OwnedFacetString& s, char c) {
    char* p;
    if (!s.small()) {
        s.words[1] = 1;
        p = (char*)s.words[2];
    } else {
        ((SmallStr8E54*)&s)->len = 1;
        p = (char*)&s + 1;
    }
    p[0] = c;
    p[1] = 0;
}
// Insert a character range at a string position.
static inline void InsertFormatRange(OwnedFacetString& s, unsigned int pos, char* first, char* last) {
    volatile unsigned int allocator;
    struct Allocator {
        unsigned char byte;
        unsigned char padding[3];
    } local;
    local.byte = *(volatile unsigned char*)&allocator;
    StdString_ReplaceRange(&s, pos, 0, first, last, (unsigned char*)&local);
}
// Append one character using the standard-string range insertion.
static inline void AppendFormatCharacter(OwnedFacetString& s, char c) {
    char buf[2];
    buf[0] = c;
    InsertFormatRange(s, s.size(), &buf[0], &buf[1]);
}
// Insert one character at the requested string position.
static inline void InsertFormatCharacter(OwnedFacetString& s, unsigned int pos, char c) {
    char buf[2];
    buf[0] = c;
    InsertFormatRange(s, pos, &buf[0], &buf[1]);
}
struct It8350 {
    char* p;
    // Wrap a borrowed character cursor.
    It8350(char* q) : p(q) {}
    // Return a cursor advanced by n bytes.
    It8350 operator+(int n) const {
        return It8350(p + n);
    }
    // Return the byte distance between two cursors in the same string.
    int operator-(const It8350& o) const {
        return p - o.p;
    }
};
// Return an iterator to the string's first character.
static inline It8350 FormatStringBegin(OwnedFacetString& s) {
    return It8350(s.data());
}
// Return an iterator just beyond the string's last character.
static inline It8350 FormatStringEnd(OwnedFacetString& s) {
    char* first;
    unsigned int len;
    if (!(s.words[0] >> 31)) {
        first = (char*)&s + 1;
        len = *(unsigned char*)&s & kInlineLengthMask;
    } else {
        first = (char*)s.words[2];
        len = s.words[1];
    }
    return It8350(first + len);
}
// Insert a one-byte range at a character iterator.
static inline void InsertFormatCharacterAtIterator(OwnedFacetString& s, It8350 pos, char* buf) {
    InsertFormatRange(s, pos - FormatStringBegin(s), &buf[0], &buf[1]);
}
// Reverse an iterator range in place.
static inline void ReverseFormatRange(It8350 first, It8350 last) {
    if (first.p != last.p) {
        while (first.p < --last.p) {
            char saved = *first.p;
            *first.p = *last.p;
            *last.p = saved;
            ++first.p;
        }
    }
}
// Lazily allocate the punctuation facet identifier for floating-point formatting.
static inline unsigned int GetFloatingPunctuationFacetId() {
    if (!s_numPunctCharFacetId)
        s_numPunctCharFacetId = ++s_localeFacetIdCounter;
    return s_numPunctCharFacetId;
}
// Find or create numeric punctuation for scientific-format output.
static inline void* GetScientificPunctuation(Loc6D9C& loc) {
    Guard6D9C guard;
    unsigned int idx = GetFloatingPunctuationFacetId();
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        Facet9C* fresh = new Facet9C;
        void** array = loc.arr;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(array, fresh, GetFloatingPunctuationFacetId());
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
struct GuardV8E54 {
    unsigned char byte;
    unsigned char padding[3];
};
// Find or create character classification for fixed-format output.
static inline void* GetFixedCtypeFacet(const Loc6D9C& loc) {
    GuardV8E54 guard;
    *(volatile unsigned char*)&guard.byte = 0;
    unsigned int idx = GetCtypeFacetId();
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        Facet6514* fresh = new Facet6514;
        void** array = loc.arr;
        unsigned int id = GetCtypeFacetId();
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(array, fresh, id);
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
// Find or create numeric punctuation for fixed-format output.
static inline void* GetFixedPunctuation(Loc6D9C& loc) {
    GuardV8E54 guard;
    *(volatile unsigned char*)&guard.byte = 0;
    unsigned int idx = GetFloatingPunctuationFacetId();
    void* f;
    if (idx >= ((unsigned int*)loc.arr)[1] || (f = ((void**)loc.arr[0])[idx]) == 0) {
        Facet9C* fresh = new Facet9C;
        void** array = loc.arr;
        ((void (*)(void*, void*, unsigned int))MWLocale_InstallFacet)(array, fresh, GetFloatingPunctuationFacetId());
        f = ((void**)loc.arr[0])[idx];
    }
    return f;
}
struct Punct8E54 {
    virtual void s0() = 0;
    virtual char point() = 0;
    virtual char separator() = 0;
};

// Round significant digits, insert the locale decimal point, and append the signed exponent.
int MWStream_FormatDoubleScientific(void* source, void* output, double value) {
    int exp;
    unsigned char* s = (unsigned char*)source;

    Loc6D9C locale(source);
    void* ctype = GetNumericCtypeFacet(locale);
    void* punct = GetScientificPunctuation(locale);
    unsigned char zero = ((CharacterWidenInterface*)ctype)->widen('0');
    char point = ((Punct8E54*)punct)->point();
    char expChar;
    if (AsMWStreamState(s)->flags & kUppercase)
        expChar = ((CharacterWidenInterface*)ctype)->widen('E');
    else
        expChar = ((CharacterWidenInterface*)ctype)->widen('e');
    OwnedFacetString& str = *(OwnedFacetString*)output;
    int precision = (*(int*)&AsMWStreamState(s)->precision);
    if (kDouble_0_StreamNumberFormatting == value) {
        AssignSingleFormatCharacter(str, zero);
        if ((AsMWStreamState(s)->flags & kShowPoint) || 0 < precision)
            AppendFormatCharacter(str, point);
        if (0 < precision)
            StdString_ReplaceRepeatedChar(&str, str.size(), 0, precision, zero);
        AppendFormatCharacter(str, expChar);
        AppendFormatCharacter(str, ((CharacterWidenInterface*)ctype)->widen('+'));
        StdString_ReplaceRepeatedChar(&str, str.size(), 0, 2, zero);
        return 0;
    }
    // Phase 2: round to precision+1 significant digits, then insert the decimal point.
    Dec8E54 dec;

    MWDecimalDigits_FromDouble(&dec, value);
    {
        OwnedFacetString rounded((Raw8E54*)0);
        ((void (*)(void*, void*, int, int*))MWDecimalDigits_RoundSignificant)(&rounded, &dec, precision + 1, &exp);
        OwnedFacetString wide((Raw8E54*)0);
        ((void (*)(void*, void*, void*))StdString_CopyConstructStorage)(&wide, ctype, &rounded);
        str.assign(wide);
    }
    unsigned int result = exp;
    int pointLength = 0;
    if ((AsMWStreamState(s)->flags & kShowPoint) || precision > 0) {
        char buf[2];
        buf[0] = point;
        InsertFormatCharacterAtIterator(str, FormatStringBegin(str) + 1, buf);
        pointLength = 1;
    }
    if ((int)str.size() < 1 + precision + pointLength)
        StdString_ReplaceRepeatedChar(&str, str.size(), 0, 1 + precision + pointLength - str.size(), zero);
    // Phase 3: append signed decimal exponent with at least two digits.
    char expCharValue = expChar;
    AppendFormatCharacter(str, expCharValue);
    if (exp < 0) {
        AppendFormatCharacter(str, ((CharacterWidenInterface*)ctype)->widen('-'));
        exp = -exp;
    } else {
        AppendFormatCharacter(str, ((CharacterWidenInterface*)ctype)->widen('+'));
    }
    if (10 > exp) {
        AppendFormatCharacter(str, ((CharacterWidenInterface*)ctype)->widen('0'));
        AppendFormatCharacter(str, ((CharacterWidenInterface*)ctype)->widen(exp + '0'));
    } else {
        int start = str.size();
        while (0 < exp) {
            int expValue = exp;
            AppendFormatCharacter(str, ((CharacterWidenInterface*)ctype)->widen(expValue % 10 + '0'));
            exp = exp / 10;
        }
        ReverseFormatRange(FormatStringBegin(str) + start, FormatStringEnd(str));
    }
    return result;
}
// Round fixed-point digits, insert fractional zeros, and apply locale integer grouping.
void MWStream_FormatDoubleFixed(void* stream, void* output, double value) {
    unsigned char* s = (unsigned char*)stream;
    Loc6D9C locale(stream);
    void* ctype = GetFixedCtypeFacet(locale);
    void* punct = GetFixedPunctuation(locale);
    char zero = ((CharacterWidenInterface*)ctype)->widen('0');
    char one = ((CharacterWidenInterface*)ctype)->widen('1');
    char point = ((Punct8E54*)punct)->point();
    int precision = (*(int*)&AsMWStreamState(s)->precision);
    OwnedFacetString& str = *(OwnedFacetString*)output;
    if (kDouble_0_StreamNumberFormatting == value) {
        AssignSingleFormatCharacter(str, zero);
        if ((AsMWStreamState(s)->flags & kShowPoint) || precision > 0)
            AppendFormatCharacter(str, point);
        char zeroValue = zero;
        StdString_ReplaceRepeatedChar(&str, str.size(), 0, precision, zeroValue);
        return;
    }
    // Phase 2: determine required significant digits from precision and exponent.
    // Values too small to retain digits use zero/one rounding branches directly.
    struct DecB8E54 {
        OwnedFacetString digits;
        short exp;
        short pad;
        // Reserve raw fixed-point decimal result storage for the conversion routine.
        DecB8E54() : digits((Raw8E54*)0) {}
    } dec;
    MWDecimalDigits_FromDouble(&dec, value);
    int exp = dec.exp;
    int digits = 1 + (precision + exp);
    if (digits < 0) {
        int p = (*(int*)&AsMWStreamState(s)->precision);
        AssignSingleFormatCharacter(str, zero);
        if ((AsMWStreamState(s)->flags & kShowPoint) || p > 0)
            AppendFormatCharacter(str, point);
        StdString_ReplaceRepeatedChar(&str, str.size(), 0, p, zero);
        return;
    }
    if (0 == digits) {
        if (exp == -1) {
            if (MWDecimalDigits_ShouldRoundUp(&dec, 0) == 1)
                AssignSingleFormatCharacter(str, one);
            else
                AssignSingleFormatCharacter(str, zero);
            if (AsMWStreamState(s)->flags & kShowPoint)
                AppendFormatCharacter(str, point);
        } else {
            AssignSingleFormatCharacter(str, zero);
            AppendFormatCharacter(str, point);
            if (exp < -2)
                StdString_ReplaceRepeatedChar(&str, str.size(), 0, -2 - exp, zero);
            if (MWDecimalDigits_ShouldRoundUp(&dec, 0) == 1)
                AppendFormatCharacter(str, one);
            else
                AppendFormatCharacter(str, zero);
        }
        return;
    }
    {
        OwnedFacetString rounded((Raw8E54*)0);
        MWDecimalDigits_RoundSignificant(&rounded, &dec, digits, &exp);
        OwnedFacetString wide((Raw8E54*)0);
        ((void (*)(void*, void*, void*))StdString_CopyConstructStorage)(&wide, ctype, &rounded);
        str.assign(wide);
    }
    // Phase 3: build leading fractional zeros or pad integer digits, then group.
    digits = precision + exp + 1;
    if (0 > exp) {
        OwnedFacetString t;
        AssignSingleFormatCharacter(t, zero);
        AppendFormatCharacter(t, point);
        if (exp < -1)
            StdString_ReplaceRepeatedChar(&t, t.size(), 0, -1 - exp, zero);
        unsigned int at = t.size();
        unsigned int len;
        char* first;
        if (!(str.words[0] >> 31)) {
            first = (char*)&str + 1;
            len = *(unsigned char*)&str & kInlineLengthMask;
        } else {
            first = (char*)str.words[2];
            len = str.words[1];
        }
        InsertFormatRange(t, at, first, first + len);
        str.assign(t);
        if (2 + precision - t.size() == 1)
            AppendFormatCharacter(str, zero);
        return;
    }
    char zeroValue2 = zero;
    StdString_ReplaceRepeatedChar(&str, str.size(), 0, digits - str.size(), zeroValue2);
    if ((AsMWStreamState(s)->flags & kShowPoint) || 0 < precision)
        StdString_ReplaceRepeatedChar(&str, exp + 1, 0, 1, point);
    OwnedFacetString grouping(punct);
    char separator = ((Punct8E54*)punct)->separator();
    NumericText_InsertGroupingSeparators(point, (signed char)separator, &grouping, &str);
}
#pragma push
#pragma use_lmw_stmw on
// Format a signed 64-bit integer, using unsigned bit-pattern conversion for octal/hex.
CheckedNumericOutput NumPut_FormatSigned64(void* self, CheckedNumericOutput* out, NumericFormattingState* s, char fill,
                                           long long value) {
    switch (s->flags & kBaseField) {
    case kOctal: {
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        int count = 0;
        char buf[0x28];

        char* p = buf;

        if (0 == value || (s->flags & kShowBase)) {
            *p = ((CharacterWidenInterface*)entry)->widen((signed char)'0');
            p = (p + 1);
            count++;
        }
        if (value)
            count += MWNumPut_ConvertUnsigned64Digits(s, (unsigned long long)value, p, entry, 0);
        int padding = 0;
        if (s->width > count)
            padding = s->width - count;
        unsigned short adjust = s->flags & kAdjustField;
        if (adjust != kLeft && adjust != kInternal)
            it = PadNumericIterator(it, padding, fill);
        CheckedNumericOutput result = it;
        if (adjust == kInternal)
            result = PadNumericIterator(it, padding, fill);
        result = WriteNumericIteratorText(result, buf, count);
        if (adjust == kLeft)
            result = PadNumericIterator(result, padding, fill);
        s->width = 0;
        return result;
    }
    case kHexadecimal: {
        int prefixCount = 0;
        char digits[0x28];
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        char prefix[2];

        if (s->flags & kShowBase) {
            char* q = (1 + prefix);
            prefix[0] = ((CharacterWidenInterface*)entry)->widen((signed char)'0');
            if (s->flags & kUppercase)
                *q = ((CharacterWidenInterface*)entry)->widen((signed char)'X');
            else
                *q = ((CharacterWidenInterface*)entry)->widen((signed char)'x');
            prefixCount += 2;
        }

        return WriteNumericParts(it, prefixCount, prefix,
                                 MWNumPut_ConvertUnsigned64Digits(s, (unsigned long long)value, digits, entry, 0),
                                 digits, s, fill);
    }
    default: {
        char digits[0x28];
        char sign;
        CheckedNumericOutput it = *out;

        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        int signCount = 0;

        if (!(value < 0) && (s->flags & kShowPositive)) {
            sign = ((CharacterWidenInterface*)entry)->widen((signed char)'+');
            signCount = 1;
        } else if (value < 0) {
            signCount = 1;
            sign = ((CharacterWidenInterface*)entry)->widen((signed char)'-');
            value = -value;
        }
        int length = MWNumPut_ConvertUnsigned64Digits(s, (unsigned long long)value, digits, entry, 0);
        return WriteNumericParts(it, signCount, &sign, length, digits, s, fill);
    }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Format a signed 32-bit integer with sign, base prefix, and width padding.
CheckedNumericOutput NumPut_FormatSignedInteger(void* self, CheckedNumericOutput* out, NumericFormattingState* s,
                                                char fill, int value) {
    switch (s->flags & kBaseField) {
    case kOctal: {
        char buf[0x28];
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));

        char* p = buf;
        int count = 0;
        if (0 == value || (s->flags & kShowBase)) {
            *p = ((CharacterWidenInterface*)entry)->widen((signed char)'0');
            p++;
            count++;
        }
        if (value != 0)
            count += ((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(s, value, p,
                                                                                                         entry, 0);
        int padding = 0;
        if (s->width > count)
            padding = s->width - count;
        unsigned short adjust = s->flags & kAdjustField;
        if (adjust != kLeft && adjust != kInternal)
            it = PadNumericIterator(it, padding, fill);
        CheckedNumericOutput result = it;
        if (adjust == kInternal)
            result = PadNumericIterator(it, padding, fill);
        result = WriteNumericIteratorText(result, buf, count);
        if (adjust == kLeft)
            result = PadNumericIterator(result, padding, fill);
        s->width = 0;
        return result;
    }
    case kHexadecimal: {
        int prefixCount = 0;
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        char prefix[2];
        char digits[0x28];

        if (s->flags & kShowBase) {
            char* q = 1 + prefix;
            prefix[0] = ((CharacterWidenInterface*)entry)->widen((signed char)'0');
            if (s->flags & kUppercase)
                *q = ((CharacterWidenInterface*)entry)->widen((signed char)'X');
            else
                *q = ((CharacterWidenInterface*)entry)->widen((signed char)'x');
            prefixCount += 2;
        }

        return WriteNumericParts(it, prefixCount, prefix,
                                 (((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(
                                     s, value, digits, entry, 0)),
                                 digits, s, fill);
    }
    default: {
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        char sign;

        unsigned int signCount = 0;
        unsigned int magnitude = (unsigned int)value;
        if (!(value < 0) && (s->flags & kShowPositive)) {
            sign = ((CharacterWidenInterface*)entry)->widen((signed char)'+');
            signCount = 1;
        } else if (value < 0) {
            signCount = 1;
            sign = ((CharacterWidenInterface*)entry)->widen((signed char)'-');
            magnitude = 0u - magnitude;
        }
        char digits[0x28];
        int length = ((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(
            s, magnitude, digits, entry, 0);
        return WriteNumericParts(it, signCount, &sign, length, digits, s, fill);
    }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Format an unsigned 32-bit integer with base prefix and width padding.
CheckedNumericOutput NumPut_FormatUnsigned32(void* self, CheckedNumericOutput* out, NumericFormattingState* s,
                                             char fill, unsigned int value) {
    switch (s->flags & kBaseField) {
    case kOctal: {
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));

        char buf[0x28];
        char* p = buf;
        int count = 0;
        if (!value || (s->flags & kShowBase)) {
            *p = ((CharacterWidenInterface*)entry)->widen((signed char)'0');
            p++;
            count += 1;
        }
        if (value)
            count += ((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(s, value, p,
                                                                                                         entry, 0);
        int padding = 0;
        CheckedNumericOutput it2 = it;
        if (s->width > count)
            padding = s->width - count;
        unsigned short adjust = s->flags & kAdjustField;
        if (adjust != kLeft && adjust != kInternal)
            it2 = PadNumericIterator(it, padding, fill);
        CheckedNumericOutput result = it2;
        if (adjust == kInternal)
            result = PadNumericIterator(it2, padding, fill);
        result = WriteNumericIteratorText(result, buf, count);
        if (adjust == kLeft)
            result = PadNumericIterator(result, padding, fill);
        s->width = 0;
        return result;
    }
    case kHexadecimal: {
        unsigned int prefixCount = 0;
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        char prefix[2];

        char digits[0x28];
        if (s->flags & kShowBase) {

            prefix[0] = ((CharacterWidenInterface*)entry)->widen((signed char)'0');
            char* q = (prefix + 1);
            if (s->flags & kUppercase)
                *q = ((CharacterWidenInterface*)entry)->widen((signed char)'X');
            else
                *q = ((CharacterWidenInterface*)entry)->widen((signed char)'x');
            prefixCount += 2;
        }
        int length = ((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(
            s, value, digits, entry, 0);
        return WriteNumericParts(it, prefixCount, prefix, length, digits, s, fill);
    }
    default: {
        CheckedNumericOutput it = *out;
        void* entry = GetNumericCtypeFacet(Loc6D9C(s));
        char digits[0x28];

        return WriteNumericParts(it, 0, digits,
                                 (((int (*)(void*, unsigned int, char*, void*, int))MWNumPut_ConvertUnsigned32Digits)(
                                     s, value, digits, entry, 0)),
                                 digits, s, fill);
    }
    }
}
#pragma pop

} // extern "C"

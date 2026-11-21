#include <decomp/static_init.h>
// Translation unit 0x80049254-0x8004A814 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {


extern float kFloat_6_2831855;
float s_resourcePathTwoPi;

// Report that this keyboard implementation does not support this capability.
int WInputDev_MakeClone() {
    return 0;
}

// Initialize this unit's full-turn angle constant.
void ResourcePath_InitFullTurnAngle() {
    s_resourcePathTwoPi = kFloat_6_2831855;
}

struct Object_8004A768 {
    virtual void method(int);
};

// Release a non-null resource through its deleting virtual method.
void Resource_Release(Object_8004A768* self) {
    if (self != 0) {
        self->method(1);
    }
}

extern int Stream_VScanFormat(void*, const char*, void*);
// Forward a CodeWarrior varargs list to the stream scanner.
int Stream_ScanFormat(void* stream, const char* format, ...) {
    struct { unsigned int argumentState; void* registerSaveArea; void* overflowArguments; } ap = { 0, 0, 0 };
    __va_setup(&ap);
    return Stream_VScanFormat(stream, format, &ap);
}

extern void appFreeUni__FPv(char*);
struct String_80049268 {
    unsigned char small[4];
    unsigned int length; // Heap-string byte length; the tagged inline form stores length in small[0].
    char* data;
    // Free only the heap representation of this owned path string.
    ~String_80049268() {
        if ((*(unsigned int*)this >> 31) != 0) appFreeUni__FPv(data);
    }
};

extern unsigned int strlen(const char*);
extern int memcmp(const char*, const char*, unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(String_80049268*, unsigned int, unsigned int, const char*, const char*, const char*);
extern void* __nwa__FUl(unsigned int);
extern void* SharedFile_Create(const char*, int);
extern unsigned char s_rawFileDefaultFlag;
extern void* __vt__8cFileRaw[11];
extern void* s_sharedCharDeleterVtable[];
extern void* __vt__Q23std11logic_error[];
extern void* __vt__Q23std12out_of_range[];
extern void __dl__FPv(void*);

struct Result_80049D3C {
    void** vtable;
    int ref1;
    int ref2;
    void* opaqueState; // Cleared during construction; its interpretation is not established here.
    void* resource;
    unsigned char flag;
};

extern void appFreeUni__FPv(char*);

extern void* __nw__FUl(unsigned int);

extern "C" void __dt__Q23std9exceptionFv(void*);
extern char* strcpy(char*, const char*);
extern void __dla__FPv(void*);

namespace std {
struct exception { void* vtable; };
struct logic_error : exception {
    char* data;
    void* control;
    // Copy exception text and establish shared ownership while guarding allocation failure.
    logic_error(const char* message) {
        vtable = (void*)__vt__Q23std11logic_error;
        char allocator = 0;
        unsigned int n = strlen(message) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        {
            char* buffer = data;
            struct Guard {
                char* text; char* alloc;
                // Delete message bytes if shared control-block construction throws.
                ~Guard() { if (text) __dla__FPv(text); }
            } guard = { buffer, &allocator };
            struct Owner { void* table; int a, b; char* text; }* o = (struct Owner*)__nw__FUl(sizeof(struct Owner));
            if (o) { o->a = 1; o->b = 1; o->table = (void*)s_sharedCharDeleterVtable; o->text = buffer; }
            control = o;
            guard.text = 0;
        }
        strcpy(data, message);
    }
    ~logic_error();
};
struct out_of_range : logic_error {
    // Construct the range exception through its owned-message base.
    out_of_range(const char* message) : logic_error(message) { vtable = (void*)__vt__Q23std12out_of_range; }
    ~out_of_range();
};
}

struct AllocChar_4948C { char c __attribute__((aligned(4))); };
// Borrow the smaller length without introducing another value copy.
inline const unsigned int& min_4948C(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Compare equal-prefix strings by their remaining lengths.
inline int compare_4948C(unsigned int a, unsigned int b) {
    if (a < b) return -1;
    if (a == b) return 0;
    return 1;
}

#include "classes/MWStringStorage.h"

struct ImageResourceStringPool {
    char dvdPrefix[6], jpgExtension[5], compareRangeMessage[26], outOfRangeType[55];
    char pngExtension[5], bmpExtension[5], tgaExtension[5], upperJpgExtension[5], gsrExtension[5];
    char reserveMessage[35], lengthErrorType[55], stringRangeMessage[27], stringLengthMessage[27];
    unsigned char reserved[7];
};
extern const ImageResourceStringPool imageResourceStrings;
typedef char ImagePool_pngOffset[((int)&((ImageResourceStringPool*)0)->pngExtension == 0x5C) ? 1 : -1];
typedef char ImagePool_gsrOffset[((int)&((ImageResourceStringPool*)0)->gsrExtension == 0x70) ? 1 : -1];
enum { kImageExtensionBytes = 4, kHeapStringTagShift = 31, kSmallStringLengthMask = 0x7F };

// Test the tagged representation without changing string ownership.
static inline unsigned int PathUsesHeap(const String_80049268* text) {
    return AsMWStringStorage(text)->flags >> kHeapStringTagShift;
}

// Borrow the character bytes following the inline length/tag byte.
static inline const char* InlinePathData(const String_80049268* text) {
    return (const char*)text + 1;
}

#pragma push
#pragma use_lmw_stmw on
// 0x80049D3C: search backwards, returning the end sentinel when no dot exists.
static inline unsigned int FindPathDot4948C(unsigned int size, const char* data, unsigned int& pos) {
    if (size != 0) {
        unsigned int last = size - 1;
        if (last < pos) pos = last;
        const char* p = data + pos;
        for (;;) {
            if (*p == '.') return p - data;
            if (p <= data) break;
            --p;
        }
    }
    return -1;
}

// Normalize supported image suffixes to .gsr, open the shared file, and optionally initialize its raw wrapper.
void* SharedFile_CreateFromDvdPath(const char* path, unsigned int, unsigned int, int openExisting, int preserveExtension) {
    // Phase 1: copy the borrowed input path into an owned, mutable string.
    String_80049268 text = {};
    AllocChar_4948C extra;
    unsigned int len = strlen(path);
    String_ReserveCapacity(&text, len);
    AllocChar_4948C copy = extra;
    String_ReplaceRange(&text, 0, 0, path, path + len, (char*)&copy);

    // Phase 2: normalize supported image suffixes unless explicitly preserved.
    if (preserveExtension == 0) {
        unsigned int dot;
        unsigned int isLong = PathUsesHeap(&text);
        {
            unsigned int size;
            const char* data;
            unsigned int pos = -1;
            if (isLong == 0) {
                size = text.small[0] & kSmallStringLengthMask;
                data = InlinePathData(&text);
            } else {
                data = text.data;
                size = text.length;
            }
            dot = FindPathDot4948C(size, data, pos);
        }
        if (dot != (unsigned int)-1) {
// A successful comparison replaces the suffix and exits the enclosing single-pass loop.
// Control flow is explicit in the macro name: a match replaces and breaks the single-pass search.
#define REPLACE_SUFFIX_AND_BREAK_IF_MATCHED(extension) { \
                const char* ext = imageResourceStrings.extension; \
                unsigned int extLen = strlen(ext); \
                unsigned int requested = kImageExtensionBytes; \
                unsigned int size; \
                const char* data; \
                if (isLong == 0) { \
                    size = text.small[0] & kSmallStringLengthMask; \
                    data = InlinePathData(&text); \
                } else { \
                    size = text.length; \
                    data = text.data; \
                } \
                if ((unsigned int)dot > size) { \
                    throw std::out_of_range(imageResourceStrings.compareRangeMessage); \
                } \
                unsigned int remain = size - (unsigned int)dot; \
                unsigned int count = min_4948C(requested, remain); \
                int cmp = memcmp(data + dot, ext, min_4948C(extLen, count)); \
                if (cmp == 0) cmp = compare_4948C(count, extLen); \
                if (cmp == 0) { \
                    String_ReplaceRange(&text, dot, kImageExtensionBytes, imageResourceStrings.gsrExtension, \
                        imageResourceStrings.gsrExtension + strlen(imageResourceStrings.gsrExtension), ""); \
                    break; \
                } \
            }
            do {
                REPLACE_SUFFIX_AND_BREAK_IF_MATCHED(jpgExtension);
                REPLACE_SUFFIX_AND_BREAK_IF_MATCHED(pngExtension);
                REPLACE_SUFFIX_AND_BREAK_IF_MATCHED(bmpExtension);
                REPLACE_SUFFIX_AND_BREAK_IF_MATCHED(tgaExtension);
                REPLACE_SUFFIX_AND_BREAK_IF_MATCHED(upperJpgExtension);
            } while (0);
#undef REPLACE_SUFFIX_AND_BREAK_IF_MATCHED
            ;
        }
    }

    // Phase 3: acquire the shared file and initialize the raw wrapper on a new open.
    unsigned int isLong = text.small[0] >> 7;
    const char* finalPath = isLong ? text.data : InlinePathData(&text);
    Result_80049D3C* result = (Result_80049D3C*)__nw__FUl(sizeof(Result_80049D3C));
    if (result) {
        result->ref1 = 0;
        result->vtable = (void**)__vt__8cFileRaw;
        result->ref2 = 0;
        result->opaqueState = 0;
        result->resource = 0;
        result->flag = 0;
        result->flag = s_rawFileDefaultFlag;
        result->resource = SharedFile_Create(finalPath, openExisting == 0);
        int ok = result->resource != 0;
        if (ok && openExisting == 0)
            ok = ((int (*)(Result_80049D3C*))result->vtable[8])(result);
        if (!ok) {
            if (result) ((void (*)(Result_80049D3C*, int))result->vtable[2])(result, 1);
            result = 0;
        }
    }
    // text's destructor frees its buffer
    return result;
}
#pragma pop
extern void appFreeUni__FPv(char*);

extern void String_ReplaceRange(String_80049268*, unsigned int, unsigned int, const char*, const char*, const char*);

extern void __dla__FPv(void*);
extern char* strcpy(char*, const char*);
extern void __throw(const char*, void*, void*);
#pragma does_not_return __throw
extern void __dt__Q23std12out_of_rangeFv();
extern "C" void __dt__Q23std9exceptionFv(void*);
extern char* strcpy(char*, const char*);
extern void __dla__FPv(void*);
struct RangeBase_4948C {
    void* table;
    char* text;
    void* owner;
    // Destroy the standard exception base without freeing stack storage.
    ~RangeBase_4948C() { __dt__Q23std9exceptionFv(this); }
};
struct ArrayGuard_4948C {
    char* text;
    char* allocator;
    // Delete temporary message bytes unless a shared owner has adopted them.
    ~ArrayGuard_4948C() { if (text) __dla__FPv(text); }
};
struct RangeOwner_4948C {
    void* table;
    int a, b;
    char* text;
    // Adopt the message array with initial strong/weak reference counts of one.
    RangeOwner_4948C(char* p) { a = 1; b = 1; table = &s_sharedCharDeleterVtable; text = p; }
};

// Replace a recognized .jpg/.png/.bmp/.tga/.JPG suffix with .gsr; leave other suffixes alone.
void ResourcePath_NormalizeImageExtension(String_80049268* self) {
    unsigned int size;
    const char* data;
    if ((PathUsesHeap(self)) == 0) {
        size = self->small[0] & kSmallStringLengthMask;
        data = InlinePathData(self);
    } else {
        size = self->length;
        data = self->data;
    }
    unsigned int dot = -1;
    if (size != 0) {
        unsigned int pos = -1;
        unsigned int last = size - 1;
        if (last < pos) pos = last;
        const char* p = data + pos;
        for (;;) {
            if (*p == '.') { dot = p - data; break; }
            if (p <= data) break;
            --p;
        }
    }
    if (dot == (unsigned int)-1) return;

// A successful comparison exits to the shared replacement below.
// An always-inline helper changes register allocation here; retain the in-function expansion.
// The match path breaks to the shared replacement; the non-match path falls through.
#define BREAK_IF_IMAGE_SUFFIX_MATCHES(extension) { \
        const char* literal = imageResourceStrings.extension; \
        unsigned int length = strlen(literal); \
        unsigned int requested = kImageExtensionBytes; \
        unsigned int size; \
        const char* data; \
        if ((PathUsesHeap(self)) == 0) { \
            size = self->small[0] & kSmallStringLengthMask; \
            data = InlinePathData(self); \
        } else { data = self->data; size = self->length; } \
        if (dot > size) { \
            char allocator = 0; \
            RangeBase_4948C error; \
            error.table = &__vt__Q23std11logic_error; \
            const char* message = imageResourceStrings.compareRangeMessage; \
            unsigned int n = strlen(message) + 1; \
            error.text = (char*)__nwa__FUl((n << 1) - n); \
            { char* buffer = error.text; \
              ArrayGuard_4948C guard = { buffer, &allocator }; \
              error.owner = new RangeOwner_4948C(buffer); \
              guard.text = 0; } \
            strcpy(error.text, message); \
            error.table = &__vt__Q23std12out_of_range; \
            __throw(imageResourceStrings.outOfRangeType, &error, (void*)__dt__Q23std12out_of_rangeFv); \
        } \
        unsigned int remain = size - dot; \
        unsigned int count = min_4948C(requested, remain); \
        int result = memcmp(data + dot, literal, min_4948C(length, count)); \
        if (result == 0) { \
            result = compare_4948C(count, length); \
        } \
        if (result == 0) break; \
    }
    // 0x8004948C: a matching extension joins the shared replacement below.
    do {
        BREAK_IF_IMAGE_SUFFIX_MATCHES(jpgExtension);
        BREAK_IF_IMAGE_SUFFIX_MATCHES(pngExtension);
        BREAK_IF_IMAGE_SUFFIX_MATCHES(bmpExtension);
        BREAK_IF_IMAGE_SUFFIX_MATCHES(tgaExtension);
        BREAK_IF_IMAGE_SUFFIX_MATCHES(upperJpgExtension);
#undef BREAK_IF_IMAGE_SUFFIX_MATCHES
        return;
    } while (0);
    char mode;
    const char* replacement = imageResourceStrings.gsrExtension;
    String_ReplaceRange(self, dot, kImageExtensionBytes, replacement, replacement + strlen(replacement), &mode);
}

// Prepend dvd:/ unless the string already begins with that prefix, preserving the original path.
void FilePath_EnsureDvdPrefix(String_80049268* self) {
    // Phase 1: compare a prefix-sized slice and keep an existing dvd:/ path unchanged.
    unsigned int compareLength;
    unsigned int selfLength;
    unsigned int literalLength2;
    unsigned int literalLength;
    const char* lit = imageResourceStrings.dvdPrefix;
    unsigned int firstLength = strlen(lit);
    unsigned int secondLength = strlen(lit);
    literalLength2 = secondLength;
    literalLength = firstLength;

    unsigned int rawLength;
    const char* selfData;
    if ((PathUsesHeap(self)) == 0) {
        rawLength = self->small[0] & kSmallStringLengthMask;
        selfData = InlinePathData(self);
    } else {
        selfData = self->data;
        rawLength = self->length;
    }
    selfLength = rawLength;

    compareLength = min_4948C(literalLength, selfLength);
    int cmp = memcmp(selfData, lit, min_4948C(literalLength2, compareLength));
    int result;
    if (cmp == 0) {
        if (compareLength < secondLength) result = -1;
        else if (compareLength == secondLength) result = 0;
        else result = 1;
    } else {
        result = cmp;
    }
    if (result == 0) return;

    // Phase 2: preserve the original path; heap storage is deep-copied.
    AllocChar_4948C extra0;
    String_80049268 text;
    if ((PathUsesHeap(self)) == 0) {
        unsigned int secondWord = self->length;
        AsMWStringStorage(&text)->flags = AsMWStringStorage(self)->flags;
        text.length = secondWord;
        text.data = self->data;
    } else {
        AsMWStringStorage(&text)->flags = 0;
        text.length = 0;
        text.data = 0;
        String_ReserveCapacity(&text, self->length);
        AllocChar_4948C copy0 = extra0;
        String_ReplaceRange(&text, 0, 0, self->data, self->data + self->length, (char*)&copy0);
    }

    unsigned int size;
    if ((PathUsesHeap(self)) == 0) size = self->small[0] & kSmallStringLengthMask;
    else size = self->length;

    // Phase 3: replace the destination with dvd:/, then append the preserved path.
    AllocChar_4948C extra1;
    AllocChar_4948C copy1 = extra1;
    String_ReplaceRange(self, 0, size, lit, lit + strlen(lit), (char*)&copy1);

    const char* textData;
    if ((PathUsesHeap(&text)) == 0) {
        textData = InlinePathData(&text);
    } else {
        textData = text.data;
    }

    unsigned int selfSize2;
    if ((PathUsesHeap(self)) == 0) selfSize2 = self->small[0] & kSmallStringLengthMask;
    else selfSize2 = self->length;

    AllocChar_4948C extra2;
    AllocChar_4948C copy2 = extra2;
    String_ReplaceRange(self, selfSize2, 0, textData, textData + strlen(textData), (char*)&copy2);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FC8", _ctors_u80049254_0, ResourcePath_InitFullTurnAngle);
#pragma pop

#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
#include "classes/FontAtlas.h"
#include "game/string.h"
#include <decomp/cw_bitfields.h>

// Preserve the game exception identities while keeping construction adapters local.
namespace Port2ExceptionRuntime { extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn, nothrow));
void __dt__Q23std12length_errorFv();
void __dt__Q23std12out_of_rangeFv();
} }
// Translation unit 0x800A7E64-0x800A8920 (17 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// This load path keeps word access because typed pointer fields change alias scheduling.
enum FontAtlasWord {
    kFontBufferWord = 1, kFontDataWord = 2, kFontPagesWord = 3, kFontPageCountWord = 4
};
enum FontAndStringConstants {
    kFontBufferAlignment = 32, kStringHeapFlagShift = 31, kStringInlineLengthMask = 0x7F,
    kPageVectorMaxPairs = 0x1FFFFFFF, kPageVectorGrowthLimit = 0x0AAAAAAA,
    kPageVectorHalfGrowthLimit = 0x15555554
};
struct RefObject { CW_BITS2( unsigned int refs : 16, unsigned int flags : 16) };
struct AtlasPage { unsigned int stageResource; unsigned int texture; };
struct AtlasPages { AtlasPage* data; unsigned int count; unsigned int capacity; };
// View the atlas's stored page array as resource/texture pairs.
inline AtlasPages* FontPages(void* atlas) { return (AtlasPages*)AsFontAtlas(atlas)->pages; }

void CubeResourceStorage_dtor(RefObject*, int);
void FontDataView_dtor(void*, int);
void gtl_IG3DXBuffer_Create(int, unsigned int*, int);
void* Object_GetField8(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void FontAtlas_LoadPages(unsigned int*, void*);

#pragma use_lmw_stmw on
#pragma scheduling on
// Round a byte size upward to the graphics buffer's 32-byte alignment.
inline int rounded_size(int size) { int rem = size % 32; if (rem != 0) size += 32 - rem; return size; }
// Release existing page references, allocate an aligned font buffer, copy data, and rebuild the pages.
void FontAtlas_LoadFromMemory(unsigned int* self, void* fontPathKey, const void* data, int size) {
    RefObject** entry = (RefObject**)self[kFontPagesWord];
    for (; entry != (RefObject**)self[kFontPagesWord] + self[kFontPageCountWord] * 2; entry += 2) {
        RefObject* stageResource = entry[0];
        if (stageResource) { if ((stageResource->refs = stageResource->refs - 1) == 0) CubeResourceStorage_dtor(stageResource, 1); }
        RefObject* texture = entry[1];
        if (texture) { if ((texture->refs = texture->refs - 1) == 0) CubeResourceStorage_dtor(texture, 1); }
    }
    unsigned int oldPageCount = self[kFontPageCountWord];
    self[kFontPageCountWord] -= oldPageCount;
    if (self[kFontDataWord]) FontDataView_dtor((void*)self[kFontDataWord], 1);
    int alignmentRemainder = size % kFontBufferAlignment;
    int alignedBytes = size;
    if (alignmentRemainder != 0) alignedBytes += kFontBufferAlignment - alignmentRemainder;
    gtl_IG3DXBuffer_Create(alignedBytes, self + kFontBufferWord, 0);
    memcpy(Object_GetField8(self[kFontBufferWord]), data, size);
// font.abc is big-endian on disc.
    FontAtlas_LoadPages(self, fontPathKey);
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

void Texture_GetLevelDescription(void*, int, void*);
void Engine_SetTexturePalette(int, unsigned int);
void Engine_SetTextureStageResource(int, unsigned int);
unsigned int strlen(const char*);
unsigned int String_FindSubstring(void*, const char*, unsigned int, unsigned int);
unsigned int MWString_ReverseFindSubstring(unsigned int*, const char*, unsigned int, unsigned int);

// Query level zero of the selected page; the caller must supply a valid page index.
int FontAtlas_GetPageDescriptor(unsigned int* self, void* data, unsigned int index) {
    void* entry = (void*)FontPages(self)->data[index].stageResource;
    if (!entry)
        return 0;
    Texture_GetLevelDescription(entry, 0, data);
    return 1;
}

// 0x800A8304/0x800A8360 return the search result in r3 (blr right after the bl); FontAtlas_LoadPages compares it
// with -1 (0x800A7DC0, 0x800A7D28).
// Find a NUL-terminated substring starting at the requested character position.
unsigned int MWString_FindCString(void* self, const char* text, unsigned int value) {
    return String_FindSubstring(self, text, value, strlen(text));
}

// Search backward for a NUL-terminated substring from the requested position.
unsigned int MWString_ReverseFindCString(void* self, const char* text, unsigned int value) {
    return MWString_ReverseFindSubstring((unsigned int*)self, text, value, strlen(text));
}

// Return inline character storage or the heap buffer according to the small-string flag.
char* MWString_GetCString(unsigned int* self) {
    if ((((const NString*)self)->flags >> kStringHeapFlagShift) == 0)
        return (char*)self + 1;
    return (char*)((NString*)self)->data;
}

// Return the inline seven-bit length or the heap length.
unsigned int MWString_GetLength(const unsigned int* self) {
    if ((((const NString*)self)->flags >> kStringHeapFlagShift) == 0)
        return *(const unsigned char*)self & kStringInlineLengthMask;
    return ((NString*)self)->length;
}

struct Empty {};
void String_ReplaceRange(void*, void*, void*, const char*, const char*, Empty);

// Validate the page index and bind its texture and stage resource.
int FontAtlas_BindPage(unsigned int* self, int index) {
    if (index < 0 || (unsigned int)index >= FontPages(self)->count)
        return 0;
    AtlasPage* entry = FontPages(self)->data + index;
    Engine_SetTexturePalette(0, entry->texture);
    Engine_SetTextureStageResource(0, entry->stageResource);
    return 1;
}

// Replace the complete current string with the supplied C string.
void MWString_AssignCString(unsigned int* self, const char* text) {
    struct { char source; char padding[3]; char argument; } local;
    unsigned int length;
    if ((((const NString*)self)->flags >> kStringHeapFlagShift) == 0)
        length = *(unsigned char*)self & kStringInlineLengthMask;
    else
        length = ((NString*)self)->length;
    local.argument = local.source;
    ((void (*)(void*, void*, void*, const char*, const char*, void*))String_ReplaceRange)(self, 0, (void*)length, text, text + strlen(text), &local.argument);
}

// Append the supplied C string after the current content.
void MWString_AppendCString(unsigned int* self, const char* text) {
    struct { char source; char padding[3]; char argument; } local;
    unsigned int length;
    if ((((const NString*)self)->flags >> kStringHeapFlagShift) == 0)
        length = *(unsigned char*)self & kStringInlineLengthMask;
    else
        length = ((NString*)self)->length;
    local.argument = local.source;
    ((void (*)(void*, void*, void*, const char*, const char*, void*))String_ReplaceRange)(self, (void*)length, 0, text, text + strlen(text), &local.argument);
}

// Erase the requested range using an empty replacement range.
void MWString_EraseRange(void* a, void* b, void* c) {
    Empty source;
    String_ReplaceRange(a, b, c, 0, 0, source);
}

// Return the font atlas's line height in pixels.
unsigned int FontAtlas_GetLineHeight(const unsigned int* self) {
    return (unsigned int)AsFontAtlas((void*)self)->lineHeight;
}

// Return the font-data reader using the original word-sized ABI.
unsigned int FontAtlas_GetFontData(const unsigned int* self) {
    return (unsigned int)AsFontAtlas(self)->fontData;
}

// Clear both resource words of an atlas page.
void FontAtlasPage_InitEmpty(unsigned int* self) {
    self[0] = 0;
    self[1] = 0;
}

#pragma optimize_for_size on
// Search backward for a counted byte sequence; clamp the start and return unsigned -1 if absent.
unsigned int MWString_ReverseFindSubstring(unsigned int* self, const char* text, unsigned int startPosition, unsigned int patternLength) {
    const char* data;
    unsigned int length;
    if ((((const NString*)self)->flags >> kStringHeapFlagShift) == 0) {
        data = (const char*)self + 1;
        length = *(unsigned char*)self & kStringInlineLengthMask;
    } else {
        data = (const char*)((NString*)self)->data;
        length = ((NString*)self)->length;
    }
    if (length >= patternLength) {
        length -= patternLength;
        if (startPosition > length) startPosition = length;
        const char* patternEnd = text + patternLength;
        const char* candidate = data + startPosition;
        for (;;) {
            const char* candidateByte = candidate;
            const char* patternByte = text;
            for (; patternByte < patternEnd; ++patternByte, ++candidateByte) {
                if ((signed char)*patternByte != (signed char)*candidateByte) goto mismatch;
            }
            return candidate - data;
        // Break/flag and inline-predicate experiments add instructions to this counted loop.
        mismatch:
            if (candidate <= data) break;
            --candidate;
        }
    }
    return -1;
}

#pragma optimize_for_size reset
void String_ReserveCapacity(void*, unsigned int);

// Initialize an empty small string, reserve space, and copy the supplied text.
unsigned int* MWString_ConstructCString(unsigned int* self, const char* text) {
    struct { unsigned char source; char padding[3]; unsigned char argument; } local;
    ((NString*)self)->flags = 0;
    ((NString*)self)->length = 0;
    ((NString*)self)->data = 0;
    unsigned int length = strlen(text);
    String_ReserveCapacity(self, length);
    const char* end = text + length;
    local.argument = local.source;
    ((void (*)(void*, void*, void*, const char*, const char*, void*))String_ReplaceRange)(self, 0, 0, text, end, &local.argument);
    return self;
}

// Copy inline storage directly, or allocate/copy heap text for an independently owned string.
unsigned int* MWString_CopyConstruct(unsigned int* self, const unsigned int* other) {
    struct { unsigned char source; char padding[3]; unsigned char argument; } local;
    // Inline strings occupy all three words; copy their representation without interpreting the overlaid bytes.
    if ((other[0] >> kStringHeapFlagShift) == 0) {
        self[0] = other[0];
        self[1] = other[1];
        self[2] = other[2];
    } else {
        unsigned int length = other[1];
        self[0] = 0;
        self[1] = 0;
        self[2] = 0;
        String_ReserveCapacity(self, length);
        local.argument = local.source;
    ((void (*)(void*, void*, void*, const char*, const char*, void*))String_ReplaceRange)(self, 0, 0, (const char*)other[2], (const char*)(other[2] + other[1]), &local.argument);
    }
    return self;
}

} // extern "C"



























extern "C" {
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], texturePathStrings[];
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
char* strcpy(char*, const char*);
void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void __dt__Q23std12length_errorFv(void*, int);
void __throw(const char*, void*, void (*)(void*,int)) __attribute__((noreturn));
void* __nw__FUl(unsigned int);
}
#pragma exceptions on
#pragma inline_bottom_up on

#pragma use_lmw_stmw on
#pragma scheduling on
inline void* operator new(new_size_t, void* p) { return p; }
struct AllocMarker { AllocMarker() { *(volatile char*)this=0; } };
struct GrowGuard {
    void* data; unsigned int size, capacity; unsigned int* alloc; unsigned int start;
    GrowGuard(unsigned int* a) : data(0), size(0), capacity(0), alloc(a), start(0) {}
    ~GrowGuard() { if (data) { AllocMarker alloc; size -= size; appFreeUni__FPv(data); } }
};
struct ErrorGuard_u800A7E64 {
    void* ptr; void* alloc;
    ErrorGuard_u800A7E64(void* p, void* a) : ptr(p), alloc(a) {}
    ~ErrorGuard_u800A7E64() { if (ptr) __dla__FPv(ptr); }
};
#pragma scheduling on
struct ErrorCount { void* vtable; int a,b; void* text;
    ErrorCount(void* p) { a=1; b=1; vtable=s_sharedCharDeleterVtable; text=p; }
};
#pragma scheduling on
namespace std {
struct exception {
    void* vtable;
    exception() : vtable(__vt__Q23std11logic_error) {}
    ~exception();
};
struct logic_error_u800A7E64 : exception {
    char* text; ErrorCount* count;
    logic_error_u800A7E64(register const char* msg) {
        AllocMarker alloc;

        unsigned int len=strlen(msg)+1;
        char* saved = (char*)__nwa__FUl(len*2-len);
        text = saved;
        {
        ErrorGuard_u800A7E64 guard(saved, (void*)&alloc);
        count = new ErrorCount(saved);
        guard.ptr=0;
        }
        strcpy(text,msg);
    }
    ~logic_error_u800A7E64() { __dt__Q33std6detail16char_array_guardFv(this,-1); }
};
struct length_error : logic_error_u800A7E64 {
    length_error(const char* msg) : logic_error_u800A7E64(msg) { vtable=__vt__Q23std12length_error; }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
typedef std::length_error LengthError;
// Throw the supplied length error with the game's exception construction ABI.
inline void throwLength(const char* msg) {
    do { union { double align; unsigned char bytes[sizeof(LengthError)]; } port2Storage; LengthError* port2Error = (LengthError*)port2Storage.bytes; port2Error->__ct(msg); Port2ExceptionRuntime::__throw("!std::exception!!std::logic_error!!std::length_error!!", port2Error, (void*)Port2ExceptionRuntime::__dt__Q23std12length_errorFv); } while (0);
}

struct GrowPair { unsigned int a,b; GrowPair(const GrowPair& x) { try { a=x.a; b=x.b; } catch (...) { throw; } } };
struct GrowVector { GrowPair* data; unsigned int size,capacity; };
// Choose the larger page-vector growth amount by reference.
inline const unsigned int& growMax(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
#pragma optimization_level 4
#pragma opt_common_subs off
// Append a resource pair, growing and copying the vector with exception-safe temporary ownership.
extern "C" void FontAtlasPageVector_PushBack(GrowVector* self, const GrowPair* value) {
    AllocMarker alloc;
    if (self->size < self->capacity) {
        self->data[(++self->size)-1] = *value;
    } else {
        AllocMarker a,b;
        if (kPageVectorMaxPairs-self->capacity < 1) throwLength(texturePathStrings);
        unsigned int extra;
        GrowGuard guard(&self->capacity);
        extra=self->size+1-self->capacity;
        unsigned int oldcap=self->capacity;
        if (extra > kPageVectorMaxPairs-oldcap) throwLength(texturePathStrings);
        unsigned int newcap;
        if (oldcap < kPageVectorGrowthLimit) newcap=oldcap+growMax((oldcap+1)*3/5,extra);
        else if (oldcap < kPageVectorHalfGrowthLimit) newcap=oldcap+growMax((oldcap+1)/2,extra);
        else newcap=kPageVectorMaxPairs;
        AllocMarker c;
        if (newcap > kPageVectorMaxPairs) throwLength(texturePathStrings);
        GrowPair* data=(GrowPair*)appMallocDummy__FUl(newcap*8);
        unsigned int oldsize=self->size;
        GrowPair* dest=data+oldsize;
        guard.data=data;
        guard.capacity=newcap;
        dest[guard.size]=*value;
        guard.start=oldsize;
        ++guard.size;
        GrowPair* begin=self->data;
        GrowPair* end=begin+self->size;
        while (end>begin) {
            --dest; --end;
            new (dest) GrowPair(*end);
            --guard.start; ++guard.size;
        }
        GrowPair* old=self->data;
        GrowPair* fresh=(GrowPair*)guard.data;
        unsigned int size=guard.size;
        self->size=0;
        unsigned int cap=self->capacity;
        guard.data=old;
        AllocMarker swapalloc;
        self->capacity=guard.capacity;
        guard.capacity=cap;
        self->data=fresh;
        self->size=size;
        guard.size=0;
    }
}
#pragma opt_common_subs reset
#pragma optimization_level reset




































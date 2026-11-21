#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "game/string.h"
#include "game/exception.h"

// Translation unit 0x8026E834-0x8026FEF0 (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));

#include "sdk_protos.h"
#include "classes/CPreloadPartsAsync.h"
#include "classes/CPreloadAvatarAsync.h"
#include "iff/IffStruct.h"
#include "classes/CPreloadFileAsync.h"

#define AsCPreloadFileAsync(p) ((CPreloadFileAsync*)(p))

#define AsCPreloadPartsAsync(p) ((CPreloadPartsAsync*)(p))

static const int kPreloadPending = 0;
static const int kPreloadComplete = 1;
static const int kPartTextureCount = 3;
static const int kPetAlternateExtensionOffset = 6;
enum PreloadStorageLimits { kPreloadTexturePathOffset = 0x84,
    kPreloadMaxPointerCount = 0x3FFFFFFF, kPreloadThreeFifthsLimit = 0x15555555,
    kPreloadHalfLimit = 0x2AAAAAAA };
// Three word vector storage; handles move by value and the list owns appended loaders.
struct PreloadHandleVectorView { unsigned int data, size, capacity; };
struct PreloadLoaderListView { void* vtable; PreloadHandleVectorView handles; };
enum PreloadStringConstants { kHeapStringFlagShift = 31, kInlineStringLengthMask = 0x7F,
    kDecimalRadix = 10 };
static const unsigned int kStringPositionAbsent = 0xFFFFFFFFU;

struct PetPathStringPool {
    char petExtension[6];
    char vectorLengthError[20];
};
struct PreloadSelectionEntry { unsigned int typeId; void* item; };
struct PreloadSelection {
    char pad_000[5];
    unsigned char count;
    char pad_006[2];
    PreloadSelectionEntry entries[1];
};

extern "C" {

extern "C" float kFloat_6_2831855;
extern "C" float s_twoPi_8026F38C;
extern "C" float s_twoPi_8026F52C;
extern "C" float s_twoPi_8026F668;
extern "C" float s_twoPi_8026FEE4;
extern "C" void* g_resourceManager;
extern "C" void* Resource_OpenResolvedPath(void*, void*, int, int, int, int);
extern "C" int CPreloadFile_QueryStreamSlot7(void*);
extern "C" void CPreloadFile_QueryStreamSlot6(void*);
extern "C" void CPreloadFile_dtor(void*, int);
extern "C" void OwnedObjectList_UpdateAll(void*);
extern "C" int OwnedObjectList_AreAllComplete(void*);
extern "C" void* CPreloadPartsAsync_Create(void*, void*);
extern "C" void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
extern "C" void CPreloadParent_dtor(void*, int);
extern "C" void __dl__FPv(void*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void appFreeUni__FPv(void*);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);
extern "C" void __dla__FPv(void*);
extern "C" void __dt__Q23std12length_errorFv(void*);
extern "C" char __vt__Q23std11logic_error[];
extern "C" char s_sharedCharDeleterVtable[];
extern "C" char __vt__Q23std12length_error[];
extern "C" void CPreloadFile_Ctor(void*);
extern "C" void CPreloadParent_Ctor(void*);
extern "C" void ResolvedItemSelection_Clear(void*);
extern "C" int ItemSelection_ResolvePartData(void*, void*);
extern "C" void ResolvedItemSelection_Destroy(void*, int);
extern "C" void* CPreloadTextureAsync_Create(const char*);
extern "C" unsigned int String_ReserveCapacity(void*, unsigned int);
extern "C" void PreloadHandleVector_Append(void*, void*);
extern "C" void MWString_ConstructCString(void*, const char*);
extern "C" void MWString_AppendCString(void*, const char*);
extern "C" void* MWString_GetCString(void*);
extern "C" void* CPreloadWPuppetAsync_Create(void*);

extern "C" void* PreloadList_GetEntry(void*, unsigned int);
extern "C" void String_AssignCStringRange(void*, const char*);
extern "C" char petModelExtensionStrings[];
void __throw(const char*, void*, void*) __attribute__((noreturn));extern "C" char __vt__17CPreloadFileAsync[];
extern "C" char __vt__16CPreloadFileSync[];
extern "C" char __vt__18CPreloadPartsAsync[];
extern "C" char petPathStrings[];

// Synchronous loads have no deferred update work.
int CPreloadFileSync_UpdatePreloadState(void) {
    return 1;
}

// Synchronous loads always report completion.
int CPreloadFileSync_IsComplete(void) {
    return 1;
}

struct Rep_ {
    void* vt; unsigned int rc; unsigned int controlWord08; char* data;
    // Start a one-reference exception-message representation owning p.
    Rep_(char* p) { rc = 1; controlWord08 = 1; vt = s_sharedCharDeleterVtable; data = p; }
};
struct Str8_ {
    char* data; Rep_* rep;
    // Guard the message array until its representation accepts ownership.
    Str8_(char* p, char* a) : data(p), rep((Rep_*)a) {}
    // Delete the message array unless construction transferred it away.
    ~Str8_() { if (data) __dla__FPv(data); }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
namespace std { struct logic_error : exception {
    char* data; Rep_* rep;
    // Install the exception's current construction-stage vtable.
    void SetTable(void* table) { *(void**)this = table; }
    // Initialize the logic-error base interface.
    logic_error() { SetTable(__vt__Q23std11logic_error); }
    ~logic_error();
}; }
namespace std {
struct length_error : logic_error {
    // Copy an owned length-error message with the matching allocator-tag ABI.
    length_error(char* msg, char* alloc) {
        unsigned int n = strlen(msg) + 1;
        char* buf = (char*)__nwa__FUl(n * 2 - n);
        data = buf;
        {
            Str8_ tmp(buf, alloc);
            rep = new Rep_(buf);
            tmp.data = 0;
        }
        strcpy(data, msg);
        SetTable(__vt__Q23std12length_error);
    }
    ~length_error();
};
}
struct LengthErrorAllocTag { char value; /* Initialize the allocator tag. */ LengthErrorAllocTag() : value(0) {} };
// Return the greater growth request without changing either operand's lifetime.
inline const unsigned int& MaxUnsigned(const unsigned int& a, const unsigned int& b) {
    return b < a ? a : b;
}
// Temporary new-storage guard for vector growth (name kept: it is part of the mangled destructor symbol).
struct Grow834_ {
    void** data;
    unsigned int size;
    unsigned int cap;
    unsigned int* owner;
    unsigned int offset;
    // Begin an empty temporary vector allocation guard.
    Grow834_(unsigned int* p) : data(0), size(0), cap(0), owner(p), offset(0) {}
    // Free backing storage only; loader pointers have been transferred, not retained.
    ~Grow834_() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

struct String834_ {
    union {
        char storage[12];
        unsigned int words[3];
        struct { unsigned int flags, length; char* text; } heap;
        struct { unsigned char flagsAndLength; char text[11]; } inPlace;
    };
    // Construct an empty inline string.
    String834_() { words[0] = 0; words[1] = 0; words[2] = 0; }
    // Free characters only for heap-backed strings.
    ~String834_() { if (heap.flags >> kHeapStringFlagShift) appFreeUni__FPv(heap.text); }
    // Borrow the mutable inline or heap character buffer.
    char* data() { return (heap.flags >> kHeapStringFlagShift) == 0 ? inPlace.text : heap.text; }
    // Find c at or before pos, or return unsigned -1 when no character matches.
    unsigned int rfind(char c, unsigned int pos) {
        char* text;
        unsigned int size;
        if ((heap.flags >> kHeapStringFlagShift) == 0) {
            text = inPlace.text;
            size = inPlace.flagsAndLength & kInlineStringLengthMask;
        } else {
            text = heap.text;
            size = heap.length;
        }
        if (size != 0) {
            if (size - 1 < pos) pos = size - 1;
            char* p = text + pos;
            for (;;) {
                if (*p == c) return p - text;
                if (p <= text) break;
                --p;
            }
        }
        return -1;
    }
};

// Preserve the lexical throw/allocator scope. Extracting the enclosing growth
// paths to an inline helper changed .text and exception relocations.
#define THROW_TOO_LONG() do { \
    LengthErrorAllocTag sbuf_; \
    char* msg_ = ((PetPathStringPool*)petPathStrings)->vectorLengthError; \
    throw std::length_error(msg_, &sbuf_.value); \
} while (0)

#pragma push
#pragma always_inline on
// Replace existing decimal positions before the extension with units/tens of digit + 1.
// Preserve non-digit characters and do not clamp the caller's requested variant.
static inline void SetPreloadFilenameVariantDigits(String834_& variantPath, unsigned int extensionPosition, int digit) {
    // Phase 3: update units then tens independently, preserving non-digit chars.
    int usesInlineCharacters = (variantPath.heap.flags >> kHeapStringFlagShift) == 0;
    signed char filenameDigit = (usesInlineCharacters ? variantPath.inPlace.text : variantPath.heap.text)[extensionPosition - 1];
    if (filenameDigit >= '0' && filenameDigit <= '9') {
        signed char value = '0' + (digit + 1) % kDecimalRadix;
        (usesInlineCharacters ? variantPath.inPlace.text : variantPath.heap.text)[extensionPosition - 1] = value;
    }
    usesInlineCharacters = !(variantPath.heap.flags >> kHeapStringFlagShift);
    filenameDigit = (usesInlineCharacters ? variantPath.inPlace.text : variantPath.heap.text)[extensionPosition - 2];
    if (filenameDigit >= '0' && filenameDigit <= '9') {
        signed char value = '0' + (digit + 1) / kDecimalRadix;
        (usesInlineCharacters ? variantPath.inPlace.text : variantPath.heap.text)[extensionPosition - 2] = value;
    }

}
#pragma pop

#pragma use_lmw_stmw on
// Queue the base texture and optional adjacent digit variants in its filename.
// path is a borrowed record with a C-string path at +0x84, not at offset zero.
// Only decimal characters in the last two positions before the final extension
// are replaced with the units/tens of digit+1; other characters remain untouched.
// No dot, or dot at index <=1: base request only. A valid dot still queues a second
// request even when no digits changed. Return 1 means attempts, not load success.
// digit is not range-checked; callers determine whether digit+1 fits two digits.
int CPreloadAvatarAsync_QueueTextureAndDigitVariant(void* list, const char* path, int digit) {
    // Phase 1: transfer a created base-texture handle into list+4 [data,size,capacity].
    // No retain occurs. Both large blocks below implement storage growth only.
    void* createdLoader = CPreloadTextureAsync_Create(path + kPreloadTexturePathOffset);
    if (createdLoader != 0) {
        PreloadHandleVectorView* v = &((PreloadLoaderListView*)list)->handles;
        unsigned int capacity;
        unsigned int size;
        size = v->size;
        capacity = v->capacity;
        if (size < capacity) {
            unsigned int ns = size + 1;
            v->size = ns;
            ((void**)v->data)[ns - 1] = createdLoader;
        } else {
            // Reserve -> construct appended slot -> transfer old slots -> swap.
            // Zeroing old slots transfers ownership; temp frees the old allocation.
            unsigned int oldcap = capacity;
            if (kPreloadMaxPointerCount - oldcap < 1) {
                THROW_TOO_LONG();
            }
            unsigned int add = v->size + 1 - v->capacity;
            Grow834_ temp(&v->capacity);
            oldcap = v->capacity;
            if (add > kPreloadMaxPointerCount - oldcap) {
                THROW_TOO_LONG();
            }
            unsigned int newcap;
            if (oldcap < kPreloadThreeFifthsLimit) {
                newcap = oldcap + MaxUnsigned(add, ((oldcap + 1) * 3) / 5);
            } else if (oldcap < kPreloadHalfLimit) {
                newcap = oldcap + MaxUnsigned(add, (oldcap + 1) / 2);
            } else newcap = kPreloadMaxPointerCount;
            if (newcap > kPreloadMaxPointerCount) {
                THROW_TOO_LONG();
            }
            void** fresh = (void**)appMallocDummy__FUl(newcap * 4);
            unsigned int oldsize = v->size;
            temp.offset = oldsize;
            temp.data = fresh;
            (fresh + oldsize)[temp.size] = createdLoader;
            temp.cap = newcap;
            temp.size = temp.size + 1;
            void** old = (void**)v->data;
            int count = (int)((char*)((void**)v->data + v->size) - (char*)old) / 4;
            temp.offset = oldsize - count;
            memcpy(fresh + temp.offset, old, count * 4);
            memset(old, 0, count * 4);
            temp.size = temp.size + count;
            void** olddata = (void**)v->data;
            unsigned int oldcapacity = v->capacity;
            v->size = 0;
            v->capacity = temp.cap;
            temp.cap = oldcapacity;
            v->data = (unsigned int)temp.data;
            temp.data = olddata;
            v->size = temp.size;
            temp.size = 0;
        }
    }
    // Phase 2: make a private path copy and locate the final extension separator.
    String834_ variantPath;
    unsigned int pathLength = strlen(path + kPreloadTexturePathOffset);
    String_ReserveCapacity(variantPath.storage, pathLength);
    struct A { char value; };
    volatile A b, a;
    a.value = b.value;
    String_ReplaceRange(variantPath.storage, 0, 0, path + kPreloadTexturePathOffset, path + kPreloadTexturePathOffset + pathLength, (void*)&a);
    unsigned int extensionPosition = variantPath.rfind('.', -1);
    if (extensionPosition > 1 && extensionPosition != kStringPositionAbsent) {
        SetPreloadFilenameVariantDigits(variantPath, extensionPosition, digit);
        char* variantFilename = ((variantPath.heap.flags >> kHeapStringFlagShift) == 0) ? variantPath.inPlace.text : variantPath.heap.text;
        // Phase 4: queue the variant using the same reserve/copy/swap mechanics.
        createdLoader = CPreloadTextureAsync_Create(variantFilename);
        if (createdLoader != 0) {
            PreloadHandleVectorView* v = &((PreloadLoaderListView*)list)->handles;
            unsigned int capacity;
            unsigned int size;
            size = v->size;
            capacity = v->capacity;
            if (size < capacity) {
                unsigned int ns = size + 1;
                v->size = ns;
                ((void**)v->data)[ns - 1] = createdLoader;
            } else {
                unsigned int oldcap = capacity;
                if (kPreloadMaxPointerCount - oldcap < 1) {
                    THROW_TOO_LONG();
                }
                unsigned int add = v->size + 1 - v->capacity;
                Grow834_ temp(&v->capacity);
                oldcap = v->capacity;
                if (add > kPreloadMaxPointerCount - oldcap) {
                    THROW_TOO_LONG();
                }
                unsigned int newcap;
                if (oldcap < kPreloadThreeFifthsLimit) {
                    newcap = oldcap + MaxUnsigned(add, ((oldcap + 1) * 3) / 5);
                } else if (oldcap < kPreloadHalfLimit) {
                    newcap = oldcap + MaxUnsigned(add, (oldcap + 1) / 2);
                } else newcap = kPreloadMaxPointerCount;
                if (newcap > kPreloadMaxPointerCount) {
                    THROW_TOO_LONG();
                }
                void** fresh = (void**)appMallocDummy__FUl(newcap * 4);
                unsigned int oldsize = v->size;
                temp.offset = oldsize;
                temp.data = fresh;
                (fresh + oldsize)[temp.size] = createdLoader;
                temp.cap = newcap;
                temp.size = temp.size + 1;
                void** old = (void**)v->data;
                int count = (int)((char*)((void**)v->data + v->size) - (char*)old) / 4;
                temp.offset = oldsize - count;
                memcpy(fresh + temp.offset, old, count * 4);
                memset(old, 0, count * 4);
                temp.size = temp.size + count;
                void** olddata = (void**)v->data;
                unsigned int oldcapacity = v->capacity;
                v->size = 0;
                v->capacity = temp.cap;
                temp.cap = oldcapacity;
                v->data = (unsigned int)temp.data;
                temp.data = olddata;
                v->size = temp.size;
                temp.size = 0;
            }
        }
    }
    return 1;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Resolve equipped parts, then queue paired textures and base/alternate puppet models.
// Borrow source; own every non-null child after append. Append copies the handle
// VALUE from its address, never the address of the stack slot. No retain/release
// occurs at these sites. Return 0 only for null or unresolvable source.
int CPreloadPartsAsync_QueueTexturesAndPuppets(void* self, void* source, int useAlt) {
    char* texturePath;
    char* pairedTexturePath;
    int partIndex;
    void* partResource;
    int textureIndex;
    char* preloaderBytes = (char*)self;
    if (source == 0)
        return 0;

    struct Path {
        char data[0xC8];
        // Construct temporary resolved-selection storage.
        Path() { ResolvedItemSelection_Clear(this); }
        // Release the temporary selection after child requests have been queued.
        ~Path() { ResolvedItemSelection_Destroy(this, -1); }
    } resolvedParts;

    if (!ItemSelection_ResolvePartData(&resolvedParts, source))
        return 0;

    for (partIndex = 0; partIndex < ((PreloadSelection*)source)->count; ++partIndex) {
        // Per-part phase 1: queue textures only when BOTH names in a pair are nonempty.
        void* entry = PreloadList_GetEntry(&resolvedParts, (unsigned char)partIndex);
        if (entry != 0) {
            partResource = ((PreloadSelectionEntry*)entry)->item;
            if (partResource != 0) {
                texturePath = ((IFF_STRUCT::sPart*)partResource)->textures[0];
                pairedTexturePath = ((IFF_STRUCT::sPart*)partResource)->secondTextures[0];
                for (textureIndex = 0; textureIndex < kPartTextureCount; ++textureIndex) {
                    if (*texturePath != 0 && *pairedTexturePath != 0) {
                        void* loaderHandle = CPreloadTextureAsync_Create(texturePath);
                        if (loaderHandle != 0) PreloadHandleVector_Append(preloaderBytes + 4, &loaderHandle);
                        loaderHandle = CPreloadTextureAsync_Create(pairedTexturePath);
                        if (loaderHandle != 0) PreloadHandleVector_Append(preloaderBytes + 4, &loaderHandle);
                    }
                    texturePath += sizeof(((IFF_STRUCT::sPart*)0)->textures[0]);
                    pairedTexturePath += sizeof(((IFF_STRUCT::sPart*)0)->secondTextures[0]);
                }

                // Per-part phase 2: model + base extension, then optional alternate
                // extension rebuilt from the original model name (not appended twice).
                struct String { char data[0x10]; /* Release model path characters. */ ~String() { ((NString*)(this))->~NString(); } } modelName;
                MWString_ConstructCString(&modelName, ((IFF_STRUCT::sPart*)partResource)->model);
                MWString_AppendCString(&modelName, petModelExtensionStrings);
                void* modelPath = (void*)MWString_GetCString(&modelName);
                void* loaderHandle = CPreloadWPuppetAsync_Create(modelPath);
                if (loaderHandle != 0) PreloadHandleVector_Append(preloaderBytes + 4, &loaderHandle);

                if (useAlt != 0) {
                    String_AssignCStringRange(&modelName, ((IFF_STRUCT::sPart*)partResource)->model);
                    MWString_AppendCString(&modelName, petModelExtensionStrings + kPetAlternateExtensionOffset);
                    void* alternateModelPath = (void*)MWString_GetCString(&modelName);
                    loaderHandle = CPreloadWPuppetAsync_Create(alternateModelPath);
                    if (loaderHandle != 0) PreloadHandleVector_Append(preloaderBytes + 4, &loaderHandle);
                }
            }
        }
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Initialize the first preloader two-pi cache at static startup.
void InitTwoPiCache_8026F38C(void) {
    s_twoPi_8026F38C = kFloat_6_2831855;
}

// Initialize the second preloader two-pi cache at static startup.
void InitTwoPiCache_8026F52C(void) {
    s_twoPi_8026F52C = kFloat_6_2831855;
}

// Initialize the third preloader two-pi cache at static startup.
void InitTwoPiCache_8026F668(void) {
    s_twoPi_8026F668 = kFloat_6_2831855;
}

// Initialize the fourth preloader two-pi cache at static startup.
void InitTwoPiCache_8026FEE4(void) {
    s_twoPi_8026FEE4 = kFloat_6_2831855;
}

// Return whether the asynchronous file loader has completed.
int CPreloadFileAsync_IsComplete(void* self) {
    int x = AsCPreloadFileAsync(self)->state;
    return x == kPreloadComplete;
}

// Return whether every child part loader has completed.
int CPreloadPartsAsync_IsComplete(void* self) {
    int x = AsCPreloadPartsAsync(self)->state;
    return x == 1;
}

// Open the borrowed resolved path with asynchronous mode; return the resource interface.
void* CPreloadFileAsync_vfunc_4(void* unusedSelf, void* resolvedPath) {
    return Resource_OpenResolvedPath(g_resourceManager, resolvedPath, -1, 1, 0, 0);
}

// Open the borrowed resolved path with synchronous mode; return the resource interface.
void* CPreloadFileSync_vfunc_4(void* unusedSelf, void* resolvedPath) {
    return Resource_OpenResolvedPath(g_resourceManager, resolvedPath, -1, 0, 0, 0);
}

// Destroy the asynchronous file-loader base and optionally delete its storage.
void* CPreloadFileAsync_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        CPreloadFile_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the synchronous file-loader base and optionally delete its storage.
void* CPreloadFileSync_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        CPreloadFile_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the owned part-loader list and optionally delete its storage.
void* CPreloadPartsAsync_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        CPreloadParent_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return a selected item entry by its byte-sized index, or null if out of range.
void* PreloadList_GetEntry(void* self, unsigned int index) {
    if (index >= ((PreloadSelection*)self)->count)
        return 0;
    return &((PreloadSelection*)self)->entries[index & 0xFF];
}

#pragma use_lmw_stmw on
// Append one loader handle, growing storage with exception-safe ownership transfer.
// value points to a caller-local pointer. Save *value, never value itself;
// existing handles move without retain/release when the allocation grows.
void PreloadHandleVector_Append(void* self, void* value) {
    // Plain C: if (size < cap) { data[size++] = *value; return; }  otherwise grow the array
    // (new cap = cap + max(needed, cap*3/5 or cap/2), clamped to kPreloadMaxPointerCount),
    // copy the old handles over, free the old block. The three length_error throws below are
    // the library's "vector length error" checks and are unreachable in practice.
    struct Vector { unsigned int* data; unsigned int size, cap; };
    Vector* v = (Vector*)self;
    unsigned int capacity;
    unsigned int size;
    size = v->size;
    capacity = v->cap;
    if (size < capacity) {
        unsigned int ns = size + 1;
        v->size = ns;
        v->data[ns - 1] = *(unsigned int*)value;
        return;
    }
    if (kPreloadMaxPointerCount - capacity < 1) {
        LengthErrorAllocTag tagFull;
        char* msg = petModelExtensionStrings + 0x10;
        throw std::length_error(msg, &tagFull.value);
    }
    unsigned int add;
    unsigned int half, three;
    Grow834_ temp(&v->cap);
    add = v->size + 1 - v->cap;
    unsigned int oldcap = v->cap;
    if (add > kPreloadMaxPointerCount - oldcap) {
        LengthErrorAllocTag tagRange;
        char* msg = petModelExtensionStrings + 0x10;
        throw std::length_error(msg, &tagRange.value);
    }
    unsigned int newcap;
    if (oldcap < kPreloadThreeFifthsLimit) {
        three = ((oldcap + 1) * 3) / 5;
        newcap = oldcap + MaxUnsigned(add, three);
    } else if (oldcap < kPreloadHalfLimit) {
        half = (oldcap + 1) / 2;
        newcap = oldcap + MaxUnsigned(add, half);
    } else newcap = kPreloadMaxPointerCount;
    if (newcap > kPreloadMaxPointerCount) {
        LengthErrorAllocTag tagOverMax;
        char* msg = petModelExtensionStrings + 0x10;
        throw std::length_error(msg, &tagOverMax.value);
    }
    void** fresh = (void**)appMallocDummy__FUl(newcap * 4);
    unsigned int oldsize = v->size;
    temp.offset = oldsize;
    (fresh + oldsize)[temp.size] = *(void**)value;
    temp.data = fresh;
    temp.cap = newcap;
    temp.size = temp.size + 1;
    int count;
    void** old = (void**)v->data;
    count = (int)((char*)(old + v->size) - (char*)old) / 4;
    unsigned int bytes = count * 4;
    temp.offset = oldsize - count;
    memcpy(fresh + temp.offset, old, bytes);
    memset(old, 0, bytes);
    temp.size = temp.size + count;
    void** olddata = (void**)v->data;
    void** newdata = temp.data;
    unsigned int oldcapacity = v->cap;
    v->size = 0;
    temp.data = olddata;
    v->cap = temp.cap;
    temp.cap = oldcapacity;
    v->data = (unsigned int*)newdata;
    v->size = temp.size;
    temp.size = 0;
}
#pragma use_lmw_stmw reset

// Poll the stream and finish it once its completion query succeeds.
// State machine: kPreloadPending (0) -> kPreloadComplete; the switch has only the pending case.
int CPreloadFileAsync_UpdatePreloadState(void* self) {
    int x = *((&AsCPreloadFileAsync(self)->state));
    switch (x) {
        case kPreloadPending:
            if (CPreloadFile_QueryStreamSlot7(self)) {
                CPreloadFile_QueryStreamSlot6(self);
                *((&AsCPreloadFileAsync(self)->state)) = kPreloadComplete;
            }
            break;
        default:
            break;
    }
    return 1;
}

// Update child loaders and latch completion when every child is ready.
int CPreloadPartsAsync_UpdatePartLoaders(void* self) {
    OwnedObjectList_UpdateAll(self);
    int t = *((&AsCPreloadPartsAsync(self)->state));
    switch (t) {
        case kPreloadPending:
            if (OwnedObjectList_AreAllComplete(self) != 0) {
                *((&AsCPreloadPartsAsync(self)->state)) = kPreloadComplete;
            }
            break;
        default:
            break;
    }
    return 1;
}

// Create from borrowed selection/init inputs, then replace the owned loader only on success.
// alternateModelFlag is a numeric switch carried by the recovered pointer-shaped ABI;
// init slot 3 (CPreloadPartsAsync_QueueTexturesAndPuppets) uses nonzero to queue alternate puppet models.
int PreloadAvatar_ReplacePartsLoader(void* self, void* selection, void* alternateModelFlag) {
    void* replacementLoader = CPreloadPartsAsync_Create(selection, alternateModelFlag);
    if (replacementLoader == 0) {
        return 0;
    }
    void* previousLoader = ((CPreloadAvatarAsync*)self)->partsPreload;
    if (previousLoader != 0) {
        if (previousLoader != 0) {
            struct V { 
            virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((V*)previousLoader)->c(1);
        }
        ((CPreloadAvatarAsync*)self)->partsPreload = 0;
    }
    ((CPreloadAvatarAsync*)self)->partsPreload = replacementLoader;
    return 1;
}

// Replace the entire small-string contents with the supplied C string.
void String_AssignCStringRange(void* self, const char* str) {
    int size;
    if ((*((unsigned int*)self) >> 31) == 0) {
        size = *((unsigned char*)self) & 0x7F;
    } else {
        size = ((NString*)self)->length;
    }
    // Allocator-tag scratch follows the recovered string ABI; these bytes are not string data.
    volatile char allocatorScratch[8];
    unsigned int len = strlen((allocatorScratch[0] = allocatorScratch[4], str));
    String_ReplaceRange(self, 0, size, str, str + len, (void*)allocatorScratch);
}

// Allocate and initialize a synchronous file loader, deleting it on initialization failure.
void* CPreloadFileSync_Create(void* arg) {
    struct Obj {
        // Construct the synchronous file-loader base and install its concrete interface.
        Obj() { CPreloadFile_Ctor(this); *(void**)this = __vt__16CPreloadFileSync; }
        char unk[0x14];
    };
    Obj* p = new Obj;
    if (!p) return 0;
    struct VObj { 
    virtual void d2(int) DECOMP_LOCAL_VIRTUAL; virtual void d3() DECOMP_LOCAL_VIRTUAL; virtual void d4() DECOMP_LOCAL_VIRTUAL; virtual int init(void*) DECOMP_LOCAL_VIRTUAL;
    };
    VObj* vp = (VObj*)p;
    if (!vp->init(arg)) {
        if (p) vp->d2(1);
        return 0;
    }
    return p;
}

// Allocate a pending asynchronous file loader and initialize its request.
void* CPreloadFileAsync_Create(void* arg) {
    struct Obj {
        // Construct the asynchronous file loader in its pending state.
        Obj() { CPreloadFile_Ctor(this); *(void**)this = __vt__17CPreloadFileAsync; AsCPreloadFileAsync(this)->state = kPreloadPending; }
        char unk[0x18];
    };
    Obj* p = new Obj;
    if (!p) return 0;
    struct VObj { 
    virtual void d2(int) DECOMP_LOCAL_VIRTUAL; virtual void d3() DECOMP_LOCAL_VIRTUAL; virtual void d4() DECOMP_LOCAL_VIRTUAL; virtual int init(void*) DECOMP_LOCAL_VIRTUAL;
    };
    VObj* vp = (VObj*)p;
    if (!vp->init(arg)) {
        if (p) vp->d2(1);
        return 0;
    }
    return p;
}

// Return a newly owned parts loader, or null after failed initialization and cleanup.
// selection is borrowed during virtual init; whether init retains it is not established.
// alternateModelFlag carries the numeric switch consumed by init slot 3; it is never dereferenced.
void* CPreloadPartsAsync_Create(void* selection, void* alternateModelFlag) {
    struct Obj {
        // Construct the asynchronous parts-loader parent in its pending state.
        Obj() { CPreloadParent_Ctor(this); *(void**)this = __vt__18CPreloadPartsAsync; AsCPreloadPartsAsync(this)->state = kPreloadPending; }
        char unk[0x14];
    };
    Obj* p = new Obj;
    if (!p) return 0;
    struct VObj { 
    virtual void d2(int) DECOMP_LOCAL_VIRTUAL; virtual void d3() DECOMP_LOCAL_VIRTUAL; virtual void d4() DECOMP_LOCAL_VIRTUAL; virtual int init(void*, void*) DECOMP_LOCAL_VIRTUAL;
    };
    VObj* vp = (VObj*)p;
    if (!vp->init(selection, alternateModelFlag)) {
        if (p) vp->d2(1);
        return 0;
    }
    return p;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462350", _ctors_u8026E834_0, InitTwoPiCache_8026F38C);
STATIC_INIT_ENTRY(".CRT$XCU80462350", _ctors_u8026E834_1, InitTwoPiCache_8026F52C);
STATIC_INIT_ENTRY(".CRT$XCU80462350", _ctors_u8026E834_2, InitTwoPiCache_8026F668);
STATIC_INIT_ENTRY(".CRT$XCU80462350", _ctors_u8026E834_3, InitTwoPiCache_8026FEE4);
#pragma pop

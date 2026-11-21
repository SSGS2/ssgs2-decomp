#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "classes/MD5Context.h"
#define AsMD5Context(p) ((MD5Context*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char vectorLengthErrorStrings_8029D128[];
}
}

#include "sdk_protos.h"
#include "classes/CDiskErrorDialog_PAL.h"

#define AsCDiskErrorDialog_PAL(p) ((CDiskErrorDialog_PAL*)(p))
// Translation unit 0x8029D128-0x8029E894 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// MD5 round functions and constants, with game-specific native-word byte order.
// Blocks are loaded as big-endian PPC words; final state words are copied directly
// to the digest. This is not the interoperable RFC 1321 byte encoding.
static inline unsigned int MD5MixF(unsigned int x, unsigned int y, unsigned int z) {
    return z ^ (x & (y ^ z));
}
static inline unsigned int MD5MixG(unsigned int x, unsigned int y, unsigned int z) {
    return y ^ (z & (x ^ y));
}
static inline unsigned int MD5MixH(unsigned int x, unsigned int y, unsigned int z) {
    return x ^ y ^ z;
}
static inline unsigned int MD5MixI(unsigned int x, unsigned int y, unsigned int z) {
    return y ^ (x | ~z);
}
// The game's MD5 reads 32-bit words of the buffer as big-endian words and stores its state words big-endian (as in
// Wii saves).
static inline void MD5Step(unsigned int& a, unsigned int b, unsigned int mixed,
                           unsigned int word, unsigned int constant, int shift) {
    a += mixed + word + constant;
    a = (a << shift) | ((a & 0xffffffff) >> (32 - shift));
    a += b;
}
enum { kMd5BlockBytes = 64, kMd5LengthBytes = 8, kMd5DigestBytes = 16,
       kDiskErrorImageCount = 3,
       kMd5PaddingStartOffset = (int)&((MD5Context*)0)->blockWords + 1 };

#pragma push
#pragma use_lmw_stmw on
// Compress one 64-byte block using native PPC words and the MD5 round schedule.
void GameMD5_Transform(void* state, void* block) {
    unsigned int* st = (unsigned int*)state;
    unsigned int* words = (unsigned int*)block;
    unsigned int a = st[0], b = st[1], c = st[2], d = st[3];

    MD5Step(a, b, MD5MixF(b, c, d), words[0], 0xd76aa478, 7);
    MD5Step(d, a, MD5MixF(a, b, c), words[1], 0xe8c7b756, 12);
    MD5Step(c, d, MD5MixF(d, a, b), words[2], 0x242070db, 17);
    MD5Step(b, c, MD5MixF(c, d, a), words[3], 0xc1bdceee, 22);
    MD5Step(a, b, MD5MixF(b, c, d), words[4], 0xf57c0faf, 7);
    MD5Step(d, a, MD5MixF(a, b, c), words[5], 0x4787c62a, 12);
    MD5Step(c, d, MD5MixF(d, a, b), words[6], 0xa8304613, 17);
    MD5Step(b, c, MD5MixF(c, d, a), words[7], 0xfd469501, 22);
    MD5Step(a, b, MD5MixF(b, c, d), words[8], 0x698098d8, 7);
    MD5Step(d, a, MD5MixF(a, b, c), words[9], 0x8b44f7af, 12);
    MD5Step(c, d, MD5MixF(d, a, b), words[10], 0xffff5bb1, 17);
    MD5Step(b, c, MD5MixF(c, d, a), words[11], 0x895cd7be, 22);
    MD5Step(a, b, MD5MixF(b, c, d), words[12], 0x6b901122, 7);
    MD5Step(d, a, MD5MixF(a, b, c), words[13], 0xfd987193, 12);
    MD5Step(c, d, MD5MixF(d, a, b), words[14], 0xa679438e, 17);
    MD5Step(b, c, MD5MixF(c, d, a), words[15], 0x49b40821, 22);

    MD5Step(a, b, MD5MixG(b, c, d), words[1], 0xf61e2562, 5);
    MD5Step(d, a, MD5MixG(a, b, c), words[6], 0xc040b340, 9);
    MD5Step(c, d, MD5MixG(d, a, b), words[11], 0x265e5a51, 14);
    MD5Step(b, c, MD5MixG(c, d, a), words[0], 0xe9b6c7aa, 20);
    MD5Step(a, b, MD5MixG(b, c, d), words[5], 0xd62f105d, 5);
    MD5Step(d, a, MD5MixG(a, b, c), words[10], 0x02441453, 9);
    MD5Step(c, d, MD5MixG(d, a, b), words[15], 0xd8a1e681, 14);
    MD5Step(b, c, MD5MixG(c, d, a), words[4], 0xe7d3fbc8, 20);
    MD5Step(a, b, MD5MixG(b, c, d), words[9], 0x21e1cde6, 5);
    MD5Step(d, a, MD5MixG(a, b, c), words[14], 0xc33707d6, 9);
    MD5Step(c, d, MD5MixG(d, a, b), words[3], 0xf4d50d87, 14);
    MD5Step(b, c, MD5MixG(c, d, a), words[8], 0x455a14ed, 20);
    MD5Step(a, b, MD5MixG(b, c, d), words[13], 0xa9e3e905, 5);
    MD5Step(d, a, MD5MixG(a, b, c), words[2], 0xfcefa3f8, 9);
    MD5Step(c, d, MD5MixG(d, a, b), words[7], 0x676f02d9, 14);
    MD5Step(b, c, MD5MixG(c, d, a), words[12], 0x8d2a4c8a, 20);

    MD5Step(a, b, MD5MixH(b, c, d), words[5], 0xfffa3942, 4);
    MD5Step(d, a, MD5MixH(a, b, c), words[8], 0x8771f681, 11);
    MD5Step(c, d, MD5MixH(d, a, b), words[11], 0x6d9d6122, 16);
    MD5Step(b, c, MD5MixH(c, d, a), words[14], 0xfde5380c, 23);
    MD5Step(a, b, MD5MixH(b, c, d), words[1], 0xa4beea44, 4);
    MD5Step(d, a, MD5MixH(a, b, c), words[4], 0x4bdecfa9, 11);
    MD5Step(c, d, MD5MixH(d, a, b), words[7], 0xf6bb4b60, 16);
    MD5Step(b, c, MD5MixH(c, d, a), words[10], 0xbebfbc70, 23);
    MD5Step(a, b, MD5MixH(b, c, d), words[13], 0x289b7ec6, 4);
    MD5Step(d, a, MD5MixH(a, b, c), words[0], 0xeaa127fa, 11);
    MD5Step(c, d, MD5MixH(d, a, b), words[3], 0xd4ef3085, 16);
    MD5Step(b, c, MD5MixH(c, d, a), words[6], 0x04881d05, 23);
    MD5Step(a, b, MD5MixH(b, c, d), words[9], 0xd9d4d039, 4);
    MD5Step(d, a, MD5MixH(a, b, c), words[12], 0xe6db99e5, 11);
    MD5Step(c, d, MD5MixH(d, a, b), words[15], 0x1fa27cf8, 16);
    MD5Step(b, c, MD5MixH(c, d, a), words[2], 0xc4ac5665, 23);

    MD5Step(a, b, MD5MixI(b, c, d), words[0], 0xf4292244, 6);
    MD5Step(d, a, MD5MixI(a, b, c), words[7], 0x432aff97, 10);
    MD5Step(c, d, MD5MixI(d, a, b), words[14], 0xab9423a7, 15);
    MD5Step(b, c, MD5MixI(c, d, a), words[5], 0xfc93a039, 21);
    MD5Step(a, b, MD5MixI(b, c, d), words[12], 0x655b59c3, 6);
    MD5Step(d, a, MD5MixI(a, b, c), words[3], 0x8f0ccc92, 10);
    MD5Step(c, d, MD5MixI(d, a, b), words[10], 0xffeff47d, 15);
    MD5Step(b, c, MD5MixI(c, d, a), words[1], 0x85845dd1, 21);
    MD5Step(a, b, MD5MixI(b, c, d), words[8], 0x6fa87e4f, 6);
    MD5Step(d, a, MD5MixI(a, b, c), words[15], 0xfe2ce6e0, 10);
    MD5Step(c, d, MD5MixI(d, a, b), words[6], 0xa3014314, 15);
    MD5Step(b, c, MD5MixI(c, d, a), words[13], 0x4e0811a1, 21);
    MD5Step(a, b, MD5MixI(b, c, d), words[4], 0xf7537e82, 6);
    MD5Step(d, a, MD5MixI(a, b, c), words[11], 0xbd3af235, 10);
    MD5Step(c, d, MD5MixI(d, a, b), words[2], 0x2ad7d2bb, 15);
    MD5Step(b, c, MD5MixI(c, d, a), words[9], 0xeb86d391, 21);

    st[0] += a;
    st[1] += b;
    st[2] += c;
    st[3] += d;
}
#pragma pop



extern int s_pal50HzTimingEnabled;
extern int s_languageIndex;
extern const float kFloat_0_DiskErrorDialog;
extern const float kDiskErrorDialogAlphaScale;
extern const float kDiskErrorDialogImageScale;
extern const float kFloat_6_2831855;
extern float s_twoPi_8029DA98;
extern float s_twoPi_8029DB44;
extern float s_twoPi_8029DB5C;
extern float s_twoPi_8029E888;
extern int FloatTween_Update(void*, float);
extern void CubeResourceStorage_dtor(void*, int);
extern void FloatTween_Start(void*, int, float, float);
extern unsigned char SCGetLanguage(void);
// Accumulate bytes unchanged and compress each completed native-word block.
extern void GameMD5_Update(void*, const char*, unsigned int);
// Append padding and native-word bit counts; copy the 16 native-order digest bytes.
extern void GameMD5_Final(void*, void*);
extern void GameMD5_Transform(void*, void*);
extern void* __vt__20CDiskErrorDialog_PAL[];
extern void* __vt__16IDiskErrorDialog[];
extern void UIRender_SetDefaultState(void);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_GetViewport(void*);
extern void Engine_SetFlexibleVertexFormat(void*);
extern void Engine_SetType7Resource(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, void*, void*, int);
extern float kDiskErrorDialogAtlasRects[];
extern void __dl__FPv(void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void* Archive_Open(const char*, int, int);
extern void Archive_GetEntryInfo(void*, int, unsigned int*);
extern int Archive_ReadEntry(void*, int, void*, unsigned int, int);
extern void Archive_Close(void*);
extern unsigned int appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void Texture_CreateFromImageDescription(void*, unsigned int, void*);
extern int Texture_GetLevelDescription(void*, int, unsigned int*);
extern const float kFloat_1_DiskErrorDialog;
extern const char diskErrorDialogTextureArchiveBegin[];
extern const char diskErrorDialogTextureArchiveEnd[];
extern const char vectorLengthErrorStrings_8029D128[];

extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void* __vt__Q23std11logic_error[];
extern void* s_sharedCharDeleterVtable[];
extern void* __vt__Q23std12length_error[];

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
struct ErrorStringOwner {
    char* data;
    void* allocator;
    ~ErrorStringOwner() { if (data) __dla__FPv(data); }
};
struct ErrorReference {
    void* vtable;
    int strong;
    int weak;
    char* data;
    ErrorReference(char* p) { strong = 1; weak = 1; vtable = s_sharedCharDeleterVtable; data = p; }
};
struct LengthErrorBase {
    void* vtable;
    char* data;
    ErrorReference* reference;
    LengthErrorBase(const char* const& message) {
        vtable = __vt__Q23std11logic_error;
        struct Allocator { char value; char padding[3]; } allocator;
        allocator.value = 0;
        unsigned int bufferedBytes = strlen(message) + 1;
        char* buffer = (char*)__nwa__FUl(bufferedBytes * 2 - bufferedBytes);
        data = buffer;
        {
            ErrorStringOwner owner = { buffer, &allocator };
            reference = new ErrorReference(buffer);
            owner.data = 0;
        }
        strcpy(data, message);
    }
    ~LengthErrorBase() { __dt__Q23std9exceptionFv(this, 0); }
};
namespace std {
class length_error : public LengthErrorBase {
public:
    length_error(const char* const& message) : LengthErrorBase(message) { vtable = __vt__Q23std12length_error; }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
struct ByteVector {
    unsigned char* begin;
    unsigned int size;
    unsigned int capacity;
    ByteVector() : begin(0), size(0), capacity(0) {}
    ~ByteVector() {
        if (begin) {
            size -= (begin + size) - begin;
            appFreeUni__FPv(begin);
        }
    }
};
struct ByteVectorAllocation {
    unsigned char* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* originalCapacity;
    unsigned int offset;
    ByteVectorAllocation(unsigned int* p) : begin(0), size(0), capacity(0), originalCapacity(p), offset(0) {}
    ~ByteVectorAllocation() {
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};
static inline const unsigned int& largerGrowth(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
static inline unsigned int byteVectorGrowth(unsigned int capacity, const unsigned int& required) {
    if (required > 0xffffffffU - capacity) {
        {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(vectorLengthErrorStrings_8029D128);
            exception_runtime::__throw(exception_runtime::vectorLengthErrorStrings_8029D128 + 0x14, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    unsigned int result;
    if (capacity < 0x55555555U) {
        result = capacity + largerGrowth((capacity + 1) * 3 / 5, required);
    } else if (capacity < 0xaaaaaaaaU) {
        result = capacity + largerGrowth((capacity + 1) >> 1, required);
    } else {
        result = 0xffffffffU;
    }
    return result;
}
static inline void checkByteVectorGrowth(unsigned int capacity, const unsigned int& required) {
    byteVectorGrowth(capacity, required);
}
// Load the selected disk-error texture from its archive and cache its dimensions.
extern "C" int CDiskErrorDialog_PAL_LoadImageTexture(void* self) {
    AsCDiskErrorDialog_PAL(self)->unk_20 = kFloat_1_DiskErrorDialog; // initial scalar; no consumer is established in this unit
    FloatTween_Start(AsCDiskErrorDialog_PAL(self)->opacityTweenPrefix, -1, kFloat_0_DiskErrorDialog, kFloat_0_DiskErrorDialog);
    void* resource = Archive_Open(diskErrorDialogTextureArchiveBegin, (int)(diskErrorDialogTextureArchiveEnd-diskErrorDialogTextureArchiveBegin), 3);
    if (!resource) return 0;
    int index = (*(int*)&AsCDiskErrorDialog_PAL(self)->archiveEntryIndex);
    if (!index) index += 5;
    unsigned int entry[0x12c / 4];
    Archive_GetEntryInfo(resource, -1, entry);
    if (index >= (int)entry[0]) return 0;
    Archive_GetEntryInfo(resource, index, entry);
    unsigned int count = entry[0x128 / 4];
    if (!count) return 0;
    ByteVector bytes;
    // Original 0x8029D128 unit: grow in place or fall through to reallocation.
    if (count > bytes.size) {
        do {
            unsigned int extra = count;
            if (extra > bytes.capacity || bytes.size > bytes.capacity - extra) {
                unsigned int required = bytes.size + extra;
                unsigned int capacity = bytes.capacity;
                checkByteVectorGrowth(capacity, required);
            } else {
                memset(bytes.begin + bytes.size, 0, extra);
                bytes.size += extra;
                break;
            }
            {
                ByteVectorAllocation allocation(&bytes.capacity);
                unsigned int capacity = bytes.capacity;
                unsigned int required = bytes.size + extra - capacity;
                unsigned int newCapacity = byteVectorGrowth(capacity, required);
                allocation.begin = (unsigned char*)appMallocDummy__FUl(newCapacity);
                allocation.capacity = newCapacity;
                allocation.offset = bytes.size;
                memset(allocation.begin + bytes.size + allocation.size, 0, extra);
                unsigned char* oldBegin = bytes.begin;
                unsigned char* oldEnd = oldBegin + bytes.size;
                unsigned int oldSize = oldEnd - oldBegin;
                allocation.size += extra;
                unsigned int offset = allocation.offset - oldSize;
                allocation.offset = offset;
                memcpy(allocation.begin + offset, oldBegin, oldSize);
                memset(oldBegin, 0, oldSize);
                unsigned int newSize = allocation.size + oldSize;
                unsigned char* oldData = bytes.begin;
                unsigned int oldCapacity = bytes.capacity;
                bytes.begin = allocation.begin;
                bytes.capacity = allocation.capacity;
                allocation.capacity = oldCapacity;
                allocation.begin = oldData;
                bytes.size = newSize;
                allocation.size = 0;
            }
        } while (0);
    } else if (count < bytes.size) {
        unsigned int remove = bytes.size - count;
        bytes.size -= remove;
    }
    Archive_ReadEntry(resource, index, bytes.begin, count, 3);
    Texture_CreateFromImageDescription(bytes.begin, count, &AsCDiskErrorDialog_PAL(self)->texture);
    AsCDiskErrorDialog_PAL(self)->textureHeight = kFloat_0_DiskErrorDialog;
    AsCDiskErrorDialog_PAL(self)->textureWidth = kFloat_0_DiskErrorDialog;
    unsigned int dims[9];
    if (Texture_GetLevelDescription((*(void**)&AsCDiskErrorDialog_PAL(self)->texture), 0, dims)) {
        AsCDiskErrorDialog_PAL(self)->textureWidth = (float)dims[7];
        AsCDiskErrorDialog_PAL(self)->textureHeight = (float)dims[8];
    }
    if (!(AsCDiskErrorDialog_PAL(self)->textureWidth > kFloat_0_DiskErrorDialog && AsCDiskErrorDialog_PAL(self)->textureHeight > kFloat_0_DiskErrorDialog)) {
        return 0;
    }
    Archive_Close(resource);
    return 1;
}
#pragma pop

static void call_vtable(void* context, unsigned int offset) {
    void** vtable = *(void***)context;
    ((void (*)(void*))vtable[offset / 4])(context);
}

struct DiskErrorVirtualCalls {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
};

// Run derived and base dialog teardown hooks, then optionally delete the dialog.
void* CDiskErrorDialog_PAL_dtor(void* self, int flag) {
    if (self != 0) {
        *(void**)self = __vt__20CDiskErrorDialog_PAL;
        ((DiskErrorVirtualCalls*)self)->slot8();
        ((DiskErrorVirtualCalls*)self)->slot6();
        if (self != 0) {
            *(void**)self = __vt__16IDiskErrorDialog;
            ((DiskErrorVirtualCalls*)self)->slot8();
            ((DiskErrorVirtualCalls*)self)->slot6();
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

#pragma dont_inline on
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_propagation off
#pragma opt_lifetimes off
void GameMD5_Final(void* digest, void* context) {
    unsigned int bufferedBytes = (AsMD5Context(context)->bitCountLow >> 3) & 0x3f;
    ((unsigned char*)AsMD5Context(context)->blockWords)[bufferedBytes] = 0x80;
    unsigned int paddingBytes = 0x3f - bufferedBytes;
    if (paddingBytes < 8) {
        memset((char*)context + bufferedBytes + kMd5PaddingStartOffset, 0, paddingBytes);
        GameMD5_Transform(context, (char*)AsMD5Context(context)->blockWords);
        memset((char*)AsMD5Context(context)->blockWords, 0, 0x38);
    } else {
        memset((char*)context + bufferedBytes + kMd5PaddingStartOffset, 0, paddingBytes - 8);
    }
    AsMD5Context(context)->blockWords[14] = AsMD5Context(context)->bitCountLow;
    AsMD5Context(context)->blockWords[15] = AsMD5Context(context)->bitCountHigh;
    GameMD5_Transform(context, (char*)AsMD5Context(context)->blockWords);
    memcpy(digest, context, kMd5DigestBytes);
    memset(context, 0, 4);
}
#pragma dont_inline off
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset

#pragma opt_loop_invariants off
#pragma opt_common_subs off
void GameMD5_Update(void* obj, const char* input, unsigned int len) {
    unsigned int old = AsMD5Context(obj)->bitCountLow;
    unsigned int next = old + (len << 3);
    AsMD5Context(obj)->bitCountLow = next;
    if (next < old) ++AsMD5Context(obj)->bitCountHigh;
    AsMD5Context(obj)->bitCountHigh += len >> 29;
    unsigned int used = (old >> 3) & 0x3f;
    if (used) {
        unsigned int room = 0x40 - used;
        char* dst = (char*)AsMD5Context(obj)->blockWords + used;
        if (len < room) {
            memcpy(dst, input, len);
            return;
        }
        memcpy(dst, input, room);
        GameMD5_Transform(obj, (char*)AsMD5Context(obj)->blockWords);
        input = input + room;
        len -= room;
    }
    while (len >= 0x40) {
        memcpy((char*)AsMD5Context(obj)->blockWords, input, 0x40);
        GameMD5_Transform(obj, (char*)AsMD5Context(obj)->blockWords);
        input = input + 0x40;
        len -= 0x40;
    }
    memcpy((char*)AsMD5Context(obj)->blockWords, input, len);
}
#pragma opt_loop_invariants reset

int EventTiming_IsDurationScalingEnabled() { return s_pal50HzTimingEnabled; }
int Language_GetIndex() { return s_languageIndex; }
// Initialize this module's first two-pi cache at static startup.
void InitTwoPiCache_8029DA98() { s_twoPi_8029DA98 = kFloat_6_2831855; }
// Initialize this module's second two-pi cache at static startup.
void InitTwoPiCache_8029DB44() { s_twoPi_8029DB44 = kFloat_6_2831855; }
void IntPair_Set(int* values, int first, int second) {
    values[0] = first;
    values[1] = second;
}
// Initialize this module's third two-pi cache at static startup.
void InitTwoPiCache_8029DB5C() { s_twoPi_8029DB5C = kFloat_6_2831855; }

// Initialize this module's fourth two-pi cache at static startup.
void InitTwoPiCache_8029E888() { s_twoPi_8029E888 = kFloat_6_2831855; }

// Select one of the three error images, ignoring out-of-range indices.
void CDiskErrorDialog_PAL_SetImageIndex(void* obj, int value) {
    if (value < 0) return;
    if (value >= kDiskErrorImageCount) return;
    AsCDiskErrorDialog_PAL(obj)->imageIndex = value;
}

// Advance the disk-error dialog's opacity tween.
// Its FloatTween begins at +4; opacity is the embedded value at +8.
int CDiskErrorDialog_PAL_UpdateOpacityTween(void* obj, float dt) {
    FloatTween_Update(AsCDiskErrorDialog_PAL(obj)->opacityTweenPrefix, dt);
    return 1;
}

// Release the texture reference and reset the opacity tween.
void CDiskErrorDialog_PAL_ReleaseTextureAndResetFade(void* obj) {
    unsigned int* p = (*(unsigned int**)&AsCDiskErrorDialog_PAL(obj)->texture);
    if (p != 0) {
        unsigned int v = *p;
        v = (((v >> 16) - 1) << 16) | (v & 0x0000FFFFu);
        *p = v;
        if ((v >> 16) == 0) CubeResourceStorage_dtor(p, 1);
        (*(unsigned int**)&AsCDiskErrorDialog_PAL(obj)->texture) = 0;
    }
    float f = kFloat_0_DiskErrorDialog;
    FloatTween_Start(AsCDiskErrorDialog_PAL(obj)->opacityTweenPrefix, -1, f, f);
}

#pragma dont_inline on
// Compute the game's native-word MD5 variant into the caller's output.
int GameMD5_Compute(void* digest, const void* data, unsigned int inputLength) {
    unsigned int context[24];
    context[0] = 0x67452301u;
    context[1] = 0xEFCDAB89u;
    context[2] = 0x98BADCFEu;
    context[3] = 0x10325476u;
    context[4] = 0;
    context[5] = 0;
    GameMD5_Update(context, (const char*)data, inputLength);
    GameMD5_Final(digest, context);
    return 1;
}
#pragma dont_inline off

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
// Draw the selected atlas image centered in the viewport with tweened alpha.
void CDiskErrorDialog_PAL_DrawImage(const void* obj) {
    int active = AsCDiskErrorDialog_PAL(obj)->texture;
    if (active == 0) return;
    int valid;
    if (AsCDiskErrorDialog_PAL(obj)->imageIndex >= 0 && AsCDiskErrorDialog_PAL(obj)->imageIndex < kDiskErrorImageCount) valid = 1;
    else valid = 0;
    if (valid == 0) return;
    float scale = AsCDiskErrorDialog_PAL(obj)->opacity;
    int alpha = (int)(kDiskErrorDialogAlphaScale * scale);
    UIRender_SetDefaultState();
    Engine_SetTextureStageResource(0, AsCDiskErrorDialog_PAL(obj)->texture);
    int index = AsCDiskErrorDialog_PAL(obj)->imageIndex;
    float* rect = (float*)((char*)kDiskErrorDialogAtlasRects + index * 16);
    float h;
    float u0;
    float w;
    w = AsCDiskErrorDialog_PAL(obj)->textureWidth;
    h = AsCDiskErrorDialog_PAL(obj)->textureHeight;
    float uv[4];
    u0 = rect[0] / w;
    uv[0] = u0;
    uv[1] = rect[1] / h;
    uv[2] = rect[2] / w;
    uv[3] = rect[3] / h;
    unsigned int scratch[6];
    Engine_GetViewport(scratch);
    struct Pair { float x, y; };
    struct PairWords { unsigned int x, y; };
    struct Rect {
        Pair start, end;
        Rect(const Pair& p, const Pair& size) {
            ((PairWords*)&end)->x = 0; ((PairWords*)&end)->y = 0;
            *(PairWords*)&start = *(const PairWords*)&p;
            end.x = p.x + size.x;
            end.y = p.y + size.y;
        }
    };
    Pair size = { kDiskErrorDialogImageScale * (rect[2] - rect[0]), kDiskErrorDialogImageScale * (rect[3] - rect[1]) };
    Pair pos = { ((float)scratch[2] - size.x) / 2.0f, ((float)scratch[3] - size.y) / 2.0f };
    Rect dst(pos, size);
    unsigned int color = 0xFFFFFF00u | (unsigned char)alpha;
    if (&dst == 0) return;
    if (uv == 0) return;
    struct Vertex { float x, y; int zero; unsigned char color[4]; float u, v; };
    Vertex vertices[4] = {};
    unsigned char c0 = (unsigned char)(color >> 24);
    unsigned char c1 = (unsigned char)(color >> 16);
    unsigned char c2 = (unsigned char)(color >> 8);
    unsigned char c3 = (unsigned char)color;
    vertices[0].x = dst.start.x; vertices[0].y = dst.start.y; vertices[0].color[0]=c0; vertices[0].color[1]=c1; vertices[0].color[2]=c2; vertices[0].color[3]=c3; vertices[0].u=uv[0]; vertices[0].v=uv[1];
    vertices[1].x = dst.end.x; vertices[1].y = dst.start.y; vertices[1].color[0]=c0; vertices[1].color[1]=c1; vertices[1].color[2]=c2; vertices[1].color[3]=c3; vertices[1].u=uv[2]; vertices[1].v=uv[1];
    vertices[2].x = dst.start.x; vertices[2].y = dst.end.y; vertices[2].color[0]=c0; vertices[2].color[1]=c1; vertices[2].color[2]=c2; vertices[2].color[3]=c3; vertices[2].u=uv[0]; vertices[2].v=uv[3];
    vertices[3].x = dst.end.x; vertices[3].y = dst.end.y; vertices[3].color[0]=c0; vertices[3].color[1]=c1; vertices[3].color[2]=c2; vertices[3].color[3]=c3; vertices[3].u=uv[2]; vertices[3].v=uv[3];
    Engine_SetFlexibleVertexFormat((void*)0x0011F005);
    Engine_SetType7Resource(0);
    void* buf = GX_GetPrimitiveCount(4, 0x98);
    DrawPrimitiveUP(0x98, buf, vertices, 0x18);
}
#pragma pop

// Compare the game's 16-byte native-word digest with the expected binary bytes.
int GameMD5_Verify(const void* data, unsigned int inputLength, const signed char* expectedDigest) {
    unsigned int context[24];
    signed char digest[kMd5DigestBytes];
    context[0] = 0x67452301u;
    context[1] = 0xEFCDAB89u;
    context[2] = 0x98BADCFEu;
    context[3] = 0x10325476u;
    context[4] = 0;
    context[5] = 0;
    GameMD5_Update(context, (const char*)data, inputLength);
    GameMD5_Final(digest, (void*)context);
    for (int i = 0; i < kMd5DigestBytes; ++i) if (digest[i] != expectedDigest[i]) return 0;
    return 1;
}

// Map Wii language IDs to the game's five-language index and disable PAL timing scaling.
void Language_InitializeFromSystemSettings() {
    int systemLanguage = SCGetLanguage();
    switch (systemLanguage) {
        case 2: s_languageIndex = 1; break;
        case 3: s_languageIndex = 2; break;
        case 4: s_languageIndex = 3; break;
        case 5: s_languageIndex = 4; break;
        case 1:
        default: s_languageIndex = 0; break;
    }
    s_pal50HzTimingEnabled = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462490", _ctors_u8029D128_0, InitTwoPiCache_8029DA98);
STATIC_INIT_ENTRY(".CRT$XCU80462490", _ctors_u8029D128_1, InitTwoPiCache_8029DB44);
STATIC_INIT_ENTRY(".CRT$XCU80462490", _ctors_u8029D128_2, InitTwoPiCache_8029DB5C);
STATIC_INIT_ENTRY(".CRT$XCU80462490", _ctors_u8029D128_3, InitTwoPiCache_8029E888);
#pragma pop

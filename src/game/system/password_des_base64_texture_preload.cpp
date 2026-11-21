#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "MSL/throw.h"
#include "MSL/error_message.h"
struct PreloadTextureView { void* vtable; void* sharedTexture; };
enum { kDESBlockBytes = 8, kDESRounds = 16, kDESWeakKeyCount = 16,
       kPasswordPayloadBytes = 8, kCaptionScriptCount = 25,
       kBase64SourceGroupBytes = 3, kBase64OutputGroupChars = 4 };
enum DESKeyResult { DESKeyOk = 0, DESKeyBadParity = -1, DESKeyWeak = -2 };
// Byte offsets into exception metadata and the existing Base64 resource pool.
enum { kLengthErrorTypeName = 0x14 };
struct PasswordUtilityStringPool {
    char exceptionAndUtilityStrings[0xB8];
    char base64Alphabet[65]; // 64 encoding characters and the terminating NUL.
};

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char vectorLengthErrorStrings_8028DFB0[];
}
}


// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm object cleanup for exceptional exits until ownership is released.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm after normal explicit cleanup or ownership transfer.
    void release() { active = false; }
    // Destroy the guarded object only while this unwind guard remains armed.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

#define GAME_NSTRING_RANGE_CONSTRUCTOR
#include "game/string.h"
#include "sdk_protos.h"
// Keep the existing byte-array ABI view for the alphabet lookup loops.
namespace nstring_access {
extern "C" char s_base64Alphabet[];
}

// Translation unit 0x8028DFB0-0x8028F4D0 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CBpCtrlHandle.h"

#define AsCBpCtrlHandle(p) ((CBpCtrlHandle*)(p))
struct VObj {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
};

struct Vtbl8 {
    virtual void c(int);
    virtual void x1();
    virtual void x2();
    virtual void x3();
    virtual int g(void*);
};

extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u8028DFB0 {
    const unsigned int* words;
    bool active;
    // Arm cleanup of the borrowed string wrapper's owned heap storage.
    explicit StringUnwindCleanup_u8028DFB0(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disarm before normal-exit string storage release.
    void release() { active = false; }
    // Release heap storage on unwind; inline strings have nothing to free.
    ~StringUnwindCleanup_u8028DFB0() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};


extern int s_validateDESKey;
extern const unsigned char desOddParityTable[];
extern unsigned char desWeakKeyTable[];
extern unsigned int desRoundRotateByTwo[];
extern const unsigned int desKeyScheduleBitTables[];
// Set weakKeyFound to 1 on a match in the weak-key table, otherwise 0.
inline void CheckWeakKey(const unsigned char*& p, const unsigned char* in, int& i, int& weakKeyFound) {
    for (i = 0; i < kDESWeakKeyCount; i++) {
        if (memcmp(p, in, kDESBlockBytes) == 0) { weakKeyFound = 1; return; }
        p += kDESBlockBytes;
    }
    weakKeyFound = 0;
}

int DES_SetKeySchedule(void*, void*);
void __ct__15CPreloadTextureFv(void*);
extern unsigned char __vt__19CPreloadTextureSync[];
int PreloadTexture_IsResourceReady(void*);
int TexturePreload_LoadSharedResource(void*, const char*);
void* CPreloadTexture_dtor(void*, int);
void* CPreloadTextureAsync_Create(void*);
int Checksum_SumBytes8(const void*, unsigned int);
int Password_EncodeEightBytePayload(void*, void*, unsigned int, void*);
int CHashPath_Exists(int, const char*);
void* CreateSharedTextureResource__FPCci(const char*, int);
void ResourcePath_NormalizeImageExtension(void*);
void appFreeUni__FPv(void*);
void CourseModelByteBuffer_Init(void*);
void CourseModelByteBuffer_ResizeZeroFilled(void*, unsigned int);
void* CourseModelByteBuffer_GetElement(void*, int);
void DES_CryptBlockBytes(void*, void*, void*, int);
int ByteVector_GetSize(void*);
void PasswordByteBuffer_Append(void*, void*);
int Base64_Encode(void*, void*);
void ByteVector_dtor(void*, int);
extern int g_resourceManager;
int ISharedResource_GetLocked(void*);
void __dl__FPv(void*);
void* __nw__FUl(unsigned long);
int CPreloadFileSync_Create(void*);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void CBpCtrl_DeleteEntryById(void*, int);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const void*, const void*, void*);

extern signed char s_bpCtrlInitialized;
extern const char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_Dialogue[];
extern NString s_base64Alphabet;
extern const PasswordUtilityStringPool passwordUtilityStrings;
extern unsigned char __vt__15CPreloadTexture[];
extern unsigned char __vt__20CPreloadTextureAsync[];
extern const float kFloat_6_2831855;
extern float s_utilityFullTurnRadiansA;
extern float s_utilityFullTurnRadiansB;
extern float s_texturePreloadFullTurnRadiansA;
extern float s_texturePreloadFullTurnRadiansB;
extern float s_texturePreloadFullTurnRadiansC;
extern const char captionUnderAreaAnimationFile[];
void CBpCtrl_CreateEntry(void*, const char*, void*, int);
int IdMap_Lookup(void*, int);
int BpObject_PollLoad(void*);
void BpObject_StartScriptByName(void*, void*, int);
void BpObject_SetEnabled(void*, int);
extern void* captionScriptNames[];
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
char* strcpy(char*, const char*);
void __dt__Q23std12length_errorFv(void*);
extern const char vectorLengthErrorStrings_8028DFB0[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct Buf {
    char* data;
    unsigned int size;
    unsigned int cap;
    ~Buf();
};

// Free the temporary byte-vector allocation when present.
inline Buf::~Buf() {
    if (data != 0) {
        appFreeUni__FPv(data);
    }
}

struct Buf5 {
    char* data;
    unsigned int size;
    unsigned int cap;
    void* alloc;
    unsigned int off;
    ~Buf5();
};

// Clear the temporary vector's size before freeing its allocation.
inline Buf5::~Buf5() {
    if (data != 0) {
        size = 0;
        appFreeUni__FPv(data);
    }
}

struct StrTmp {
    void* volatile p;
    void* volatile q;
};

// The asynchronous texture-preload initialization predicate always succeeds.
int CPreloadTextureAsync_vfunc_1() {
    return 1;
}

// Select a caption script; invalid indices release the caption entry.
// Singleton initialization is repeated at each use because entry creation may
// change the published singleton/handle state. Keep the volatile re-read.
// This call blocks until loading completes; there is no timeout/yield here.
void CBpCtrlHandle_SetScriptIndex(void* self, int id) {
    // Shared field entryId at +0 is an integer BP-entry id (-1 means none).
    // It is passed to IdMap_Lookup/DeleteEntryById, never dereferenced as a vtable.
    // The shared field name is provisional; scriptIndex is the script selection.
    if (id == -1 || id >= kCaptionScriptCount) {
        if ((*(int*)&AsCBpCtrlHandle(self)->entryId) != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv((void*)s_bpCtrl);
                    __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Dialogue);
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), (*(int*)&AsCBpCtrlHandle(self)->entryId));
            (*(int*)&AsCBpCtrlHandle(self)->entryId) = -1;
        }
        AsCBpCtrlHandle(self)->scriptIndex = -1;
    } else if (AsCBpCtrlHandle(self)->scriptIndex != id) {
        if (AsCBpCtrlHandle(self)->scriptIndex == -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv((void*)s_bpCtrl);
                    __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Dialogue);
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), captionUnderAreaAnimationFile, self, 0);
        }
        int publishedController = *(volatile int*)&((int&)Sing_805DC5F0());
        *(volatile int*)&AsCBpCtrlHandle(self)->scriptIndex = id;
        if (publishedController == 0) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv((void*)s_bpCtrl);
                __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Dialogue);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        void* captionController = (void*)((int&)Sing_805DC5F0());
        void* captionEntry = (void*)IdMap_Lookup(captionController, (*(int*)&AsCBpCtrlHandle(self)->entryId));
        // Loading owns progress; this synchronous caption operation spins until ready.
        while (BpObject_PollLoad(captionEntry) == 0) {
        }
        BpObject_StartScriptByName(captionEntry, captionScriptNames[id], 1);
        BpObject_SetEnabled(captionEntry, 1);
    }
}

extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dla__FPv(void*);
struct StrAllocation : std::detail::__message_buffer_guard {
    StrAllocation(char* p, char* a) : std::detail::__message_buffer_guard(p, a) {}
    ~StrAllocation() { dispose(); }
};
typedef std::detail::__message_control StrControl;
struct StrErrorBase {
    void* table;
    // Install the logic_error base vtable.
    StrErrorBase() { table = __vt__Q23std11logic_error; }
    // Destroy the standard exception base without deleting the stack wrapper.
    ~StrErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : StrErrorBase {
    std::__error_message<StrAllocation, StrControl> message_;
    // Construct an owned shared diagnostic string and install length_error.
    length_error() {
        const char* message = vectorLengthErrorStrings_8028DFB0;
        message_.assign(message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
// Construct and throw the vector length error using the game's exception ABI.
inline void StrThrow() {
    {
        std::__throw_exception<std::length_error>( exception_runtime::vectorLengthErrorStrings_8028DFB0 + kLengthErrorTypeName, (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
// Byte-vector allocation owner during append growth: self retains old storage until commit.
// tmp builds the new byte at oldSize, prepends the old range, then swaps buffers with self;
// its destructor consequently frees OLD storage on success and NEW storage on unwind.
struct StrBuffer {
    char* data; volatile unsigned size; unsigned capacity; unsigned* owner; unsigned offset;
    // Start an empty reallocation owner associated with the vector capacity.
    StrBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the allocation currently held by the reallocation owner.
    ~StrBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Select the larger capacity candidate by reference.
inline const unsigned& StrMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

#pragma push
#pragma use_lmw_stmw on
// Append one byte, growing the owned vector with overflow checks and exception-safe cleanup.
void PasswordByteBuffer_Append(void* selfp, void* pcp) {
    Buf* self = (Buf*)selfp;
    const unsigned char* pc = (const unsigned char*)pcp;
    unsigned int size = self->size;
    // Phase 1: append directly when spare capacity exists (pc borrows one readable byte).
    if (size < self->cap) {
        int next = 1 + size;
        *(volatile unsigned*)&self->size = next;
        unsigned char ch = *pc;
        self->data[next - 1] = ch;
        return;
    }
    // Phase 2: checked capacity growth; reserve one new byte under a temporary owner.
    unsigned int count;
    if (0xFFFFFFFFu - self->cap < 1u) StrThrow();
    StrBuffer tmp(&self->cap);
    unsigned int n = self->size + 1 - self->cap;
    unsigned int cap = self->cap;
    unsigned int bt, at;
    unsigned int newcap;
    if (n > 0xFFFFFFFFu - cap) {
        std::__throw_exception<std::length_error>( exception_runtime::vectorLengthErrorStrings_8028DFB0 + kLengthErrorTypeName, (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
    if (cap < 0x55555555u) {
        at = (3 * (cap + 1)) / 5;
        newcap = cap + StrMax(at, n);
    } else if (cap < 0xAAAAAAAAu) {
        bt = (cap + 1) / 2;
        newcap = cap + StrMax(bt, n);
    } else {
        newcap = 0xFFFFFFFFu;
    }
    // Phase 3: construct appended byte before transferring the prefix; retain aliasing order.
    tmp.data = (char*)appMallocDummy__FUl(newcap);
    unsigned int oldSize = self->size;
    unsigned int cur = tmp.size;
    tmp.capacity = newcap;
    tmp.offset = oldSize;
    memset(tmp.data + oldSize + cur, *pc, 1);
    char* oldData = self->data;
    char* last = oldData + *(volatile unsigned*)&self->size;
    count = last - oldData;
    tmp.size = tmp.size + 1;
    tmp.offset = tmp.offset - count;
    memcpy(tmp.data + tmp.offset, oldData, count);
    memset(oldData, 0, count);
    unsigned int newSize = tmp.size + count;
    oldData = self->data;
    tmp.size = newSize;
    // Phase 4: commit data/size/capacity; tmp now owns and releases the old allocation.
    unsigned int oldCap = self->cap;
    self->size = 0;
    char* t = tmp.data;
    tmp.data = oldData;
    self->cap = tmp.capacity;
    tmp.capacity = oldCap;
    self->data = t;
    self->size = newSize;
    tmp.size = 0;
}
#pragma pop

// Delete the caption BP entry and reset both handle indices.
void CBpCtrlHandle_ReleaseEntry(void* self) {
    if ((*(int*)&AsCBpCtrlHandle(self)->entryId) != -1) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv((void*)s_bpCtrl);
                __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Dialogue);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), (*(int*)&AsCBpCtrlHandle(self)->entryId));
        (*(int*)&AsCBpCtrlHandle(self)->entryId) = -1;
    }
    AsCBpCtrlHandle(self)->scriptIndex = -1;
}

struct TypeF058 {
    void* vptr;
    unsigned int field4;
    // Construct the texture-preload base and install the synchronous derived vtable.
    TypeF058() {
        __ct__15CPreloadTextureFv(this);
        *(void**)this = __vt__19CPreloadTextureSync;
    }
};

struct CPreloadTexture {
    char storage[8];
    CPreloadTexture();
};
struct TypeF420 : CPreloadTexture {
    // Install the asynchronous vtable after implicit base construction.
    TypeF420() {
        *(void**)this = __vt__20CPreloadTextureAsync;
    }
};

// Create a synchronous preloader and destroy it if resource initialization fails.
void* CPreloadTextureSync_Create(void* arg) {
    TypeF058* obj = new TypeF058;
    if (obj == 0) return 0;
    if (((Vtbl8*)obj)->g(arg) == 0) {
        if (obj != 0) ((Vtbl8*)obj)->c(1);
        return 0;
    }
    return obj;
}

float s_passwordUtilityFullTurnRadians = kFloat_6_2831855;
NString s_base64Alphabet(passwordUtilityStrings.base64Alphabet);

#pragma dont_inline on
// Initialize the base texture preloader with no shared texture handle.
void __ct__15CPreloadTextureFv(void* self) {
    (*(unsigned int*)&((PreloadTextureView*)self)->sharedTexture) = 0;
    *(void**)self = &__vt__15CPreloadTexture;
}
#pragma dont_inline off

// Clear the eight-byte password key or payload storage supplied by the caller.
void* PasswordData_ClearEightBytes(void* self) {
    memset(self, 0, kPasswordPayloadBytes);
    return self;
}

// Copy the eight-byte password payload only when the input size is sufficient.
int PasswordData_CopyEightBytes(void* dst, const void* src, unsigned int size) {
    if (size < kPasswordPayloadBytes) {
        return 0;
    }
    memcpy(dst, src, kPasswordPayloadBytes);
    return 1;
}

// Initialize this module's cached 2-pi constant during static initialization.
void InitUtilityFullTurnRadiansA() {
    s_utilityFullTurnRadiansA = kFloat_6_2831855;
}

// Initialize this module's cached 2-pi constant during static initialization.
void InitUtilityFullTurnRadiansB() {
    s_utilityFullTurnRadiansB = kFloat_6_2831855;
}

#pragma push
#pragma use_lmw_stmw on
// Check that every byte of the eight-byte DES key has the required odd parity.
inline int keyParity(const unsigned char* in) {
    for (int j = 0; j < kDESBlockBytes; ++j) {
        if (in[j] != desOddParityTable[in[j]]) return 0;
    }
    return 1;
}
// Key schedule contract: keyBytes borrows 8 bytes; roundKeys writes 16 pairs of 32-bit words.
// Validation enabled: -1 parity failure, -2 weak key, 0 success; failures leave output untouched.
// Validation disabled: every key is expanded. Output word layout is for this DES consumer,
// not an array of canonical 48-bit subkeys serialized in network order.
// Validate an optional DES key parity/weak-key policy and expand the key into 16 round subkeys.
int DES_SetKeySchedule(void* keyBytes, void* roundKeys) {
    unsigned int* roundOutput = (unsigned int*)roundKeys;
    const unsigned char* keyCursor = (const unsigned char*)keyBytes;
    int roundIndex;
    unsigned long keyHalfC, keyHalfD, swapBits, selectedCBits, packedRoundWord;
    if (s_validateDESKey != 0) {
        // Reused scratch: first parity validity, then whether a weak key was found.
        int validationResult = keyParity(keyCursor);
        if (!validationResult) {
            return DESKeyBadParity;
        }
        const unsigned char* weakKeyCursor = desWeakKeyTable;
        CheckWeakKey(weakKeyCursor, keyCursor, roundIndex, validationResult);
        if (validationResult) {
            return DESKeyWeak;
        }
    }
    keyHalfC = (unsigned long)*keyCursor++,
    keyHalfC |= (unsigned long)*keyCursor++ << 8,
    keyHalfC |= (unsigned long)*keyCursor++ << 16,
    keyHalfC |= (unsigned long)*keyCursor++ << 24;
    keyHalfD = (unsigned long)*keyCursor++,
    keyHalfD |= (unsigned long)*keyCursor++ << 8,
    keyHalfD |= (unsigned long)*keyCursor++ << 16,
    keyHalfD |= (unsigned long)*keyCursor++ << 24;
    // Phase 2: PC-1 gathers 56 non-parity bits into 28-bit C and D halves.
    // Each XOR/mask pair exchanges bit groups between or within packed words:
    // 4-bit nibbles, separated 2-bit pairs, alternating bits, then byte lanes.
    // Before the round loop, scratch names describe storage, not DES subkeys.
    swapBits = (keyHalfD >> 4) ^ keyHalfC;
    selectedCBits = swapBits & 0x0f0f0f0f;
    keyHalfC ^= selectedCBits;
    keyHalfD ^= selectedCBits << 4;
    swapBits = ((keyHalfC << 18) ^ keyHalfC) & 0xcccc0000;
    keyHalfC = keyHalfC ^ swapBits ^ (swapBits >> 18);
    swapBits = ((keyHalfD << 18) ^ keyHalfD) & 0xcccc0000;
    keyHalfD = keyHalfD ^ swapBits ^ (swapBits >> 18);
    swapBits = ((keyHalfD >> 1) ^ keyHalfC) & 0x55555555;
    keyHalfC ^= swapBits;
    keyHalfD ^= swapBits << 1;
    // Exchange byte lanes 0 and 2, preserving the intervening lane.
    swapBits = (keyHalfC >> 8) ^ keyHalfD;
    packedRoundWord = (swapBits & 0x00ff0000) | (swapBits & 0xff);
    keyHalfD ^= packedRoundWord;
    keyHalfC ^= packedRoundWord << 8;
    swapBits = ((keyHalfD >> 1) ^ keyHalfC) & 0x55555555;
    keyHalfC ^= swapBits;
    keyHalfD ^= swapBits << 1;
    // Reverse the low three D bytes and move the high C nibble into D[24..27].
    keyHalfD = ((keyHalfD & 0xff) << 16) | (keyHalfD & 0xff00) | ((keyHalfD >> 16) & 0xff) | ((keyHalfC & 0xf0000000) >> 4);
    keyHalfC &= 0x0fffffff;
    // Phase 3: rotate the 28-bit halves each round; this packed bit convention rotates right.
    // desRoundRotateByTwo selects two bits rather than one; mask off bits above each half.
    for (roundIndex = 0; roundIndex < kDESRounds; roundIndex++) {
        if (desRoundRotateByTwo[roundIndex] != 0) {
            keyHalfC = (keyHalfC >> 2) | (keyHalfC << 26);
            keyHalfD = (keyHalfD >> 2) | (keyHalfD << 26);
        } else {
            keyHalfC = (keyHalfC >> 1) | (keyHalfC << 27);
            keyHalfD = (keyHalfD >> 1) | (keyHalfD << 27);
        }
        keyHalfC &= 0x0fffffff;
        keyHalfD &= 0x0fffffff;
        // PC-2: eight 64-entry lookup slices (offsets 0,64,...,448). First four select
        // C bits, last four D bits. OR partial results, interleave 16-bit halves,
        // then rotate the two packed output words by 2 and 6 for the round-function layout.
        // Table selector bit order, least-significant selector bit first:
        // C slices: [0..5], [6,7,9..12], [13..16,18,19], [20,22,23,25..27].
        // D slices: [0..5], [7,8,10..13], [15..20], [21..24,26,27].
        // These skipped source positions implement PC-2; keep OR grouping/order.
        selectedCBits = desKeyScheduleBitTables[(((keyHalfC >> 20) & 1) | ((keyHalfC >> 21) & 6) | ((keyHalfC >> 22) & 0x38)) + 192]
            | (desKeyScheduleBitTables[keyHalfC & 0x3f]
            | desKeyScheduleBitTables[(((keyHalfC >> 6) & 3) | ((keyHalfC >> 7) & 0x3c)) + 64]
            | desKeyScheduleBitTables[(((keyHalfC >> 13) & 0x0f) | ((keyHalfC >> 14) & 0x30)) + 128]);
        swapBits = desKeyScheduleBitTables[((keyHalfD >> 15) & 0x3f) + 384]
            | (desKeyScheduleBitTables[(keyHalfD & 0x3f) + 256]
            | desKeyScheduleBitTables[(((keyHalfD >> 7) & 3) | ((keyHalfD >> 8) & 0x3c)) + 320])
            | desKeyScheduleBitTables[(((keyHalfD >> 21) & 0x0f) | ((keyHalfD >> 22) & 0x30)) + 448];
        packedRoundWord = ((swapBits & 0xffff) << 16) | (selectedCBits & 0xffff);
        selectedCBits = (selectedCBits >> 16) | (swapBits & 0xffff0000);
        roundOutput[0] = (packedRoundWord >> 30) + (packedRoundWord << 2);
        roundOutput[1] = (selectedCBits >> 26) + (selectedCBits << 6);
        roundOutput += 2;
    }
    return 0;
}
#pragma pop

// Expand the caller-supplied eight-byte key into the output DES key schedule.
int Password_SetDESKeySchedule(void* self, void* arg) {
    return DES_SetKeySchedule(self, arg);
}

// Poll whether the asynchronous preloader has a ready shared texture.
int CPreloadTextureAsync_IsResourceReady(void* self) {
    return PreloadTexture_IsResourceReady(self);
}

// The synchronous texture-preload initialization predicate always succeeds.
int CPreloadTextureSync_vfunc_1() {
    return 1;
}

// Initialize this module's cached 2-pi constant during static initialization.
void InitTexturePreloadFullTurnRadiansA() {
    s_texturePreloadFullTurnRadiansA = kFloat_6_2831855;
}

// Synchronous preloading has no pending readiness work after initialization.
int CPreloadTextureSync_vfunc_2() {
    return 1;
}

// Destroy the synchronous texture preloader and optionally delete its storage.
int CPreloadTextureSync_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        CPreloadTexture_dtor(self, 0);
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return (int)self;
}

// Load a named shared texture and wait until it is ready.
int CPreloadTextureSync_LoadSharedAndWaitReady(void* self, const char* name) {
    if (TexturePreload_LoadSharedResource(self, name) == 0) {
        return 0;
    }
    while (PreloadTexture_IsResourceReady(self) == 0) {
    }
    return 1;
}

// Treat an empty preloader as ready; otherwise poll its shared resource.
int PreloadTexture_IsResourceReady(void* self) {
    void* ptr = ((PreloadTextureView*)self)->sharedTexture;
    if (ptr == 0) {
        return 1;
    }
    return ISharedResource_GetLocked(ptr) != 0;
}

// Create the synchronous shared-file preloader requested by the caller.
int CPreloadTextureSync_CreateFileSync(void*, void* arg) {
    return CPreloadFileSync_Create(arg);
}

// Initialize this module's cached 2-pi constant during static initialization.
void InitTexturePreloadFullTurnRadiansB() {
    s_texturePreloadFullTurnRadiansB = kFloat_6_2831855;
}

// The base texture-preload initialization predicate always succeeds.
int CPreloadTexture_vfunc_1() {
    return 1;
}

// Initialize this module's cached 2-pi constant during static initialization.
void InitTexturePreloadFullTurnRadiansC() {
    s_texturePreloadFullTurnRadiansC = kFloat_6_2831855;
}

// Release the shared texture handle and optionally delete the preloader.
void* CPreloadTexture_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        *(void**)self = &__vt__15CPreloadTexture;
        void* inner = ((PreloadTextureView*)self)->sharedTexture;
        if (inner != 0) {
            ((Vtbl8*)inner)->x1();
            ((PreloadTextureView*)self)->sharedTexture = 0;
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Create a texture preloader that leaves resource readiness to subsequent polls.
void* CPreloadTextureAsync_Create(void* arg) {
    TypeF420* p = new TypeF420;
    if (p == 0) {
        return 0;
    }
    if (((int(*)(void*, void*))TexturePreload_LoadSharedResource)(p, arg) == 0) {
        if (p != 0) {
            ((Vtbl8*)p)->c(1);
        }
        return 0;
    }
    return p;
}

// DES-encrypt exactly eight bytes, append their byte-sum checksum, and Base64-encode the result.
int Password_EncodeEightBytePayload(void* self, void* src, unsigned int payloadBytes, void* encodedText) {
    if (payloadBytes != kPasswordPayloadBytes) {
        return 0;
    }
    char keySchedule[0x88];
    char plainBytes[12];
    char encryptedBytes[12];
    CourseModelByteBuffer_Init(plainBytes);
    EH5Cleanup eh5Cleanup14(plainBytes, (void (*)(void*, int))ByteVector_dtor);
    CourseModelByteBuffer_ResizeZeroFilled(plainBytes, payloadBytes);
    memcpy(CourseModelByteBuffer_GetElement(plainBytes, 0), src, payloadBytes);
    CourseModelByteBuffer_Init(encryptedBytes);
    EH5Cleanup eh5Cleanup15(encryptedBytes, (void (*)(void*, int))ByteVector_dtor);
    CourseModelByteBuffer_ResizeZeroFilled(encryptedBytes, payloadBytes);
    memset(CourseModelByteBuffer_GetElement(encryptedBytes, 0), 0, payloadBytes);
    ((int(*)(void*, void*))Password_SetDESKeySchedule)(self, keySchedule);
    void* encryptedData = CourseModelByteBuffer_GetElement(encryptedBytes, 0);
    DES_CryptBlockBytes(CourseModelByteBuffer_GetElement(plainBytes, 0), encryptedData, keySchedule, 1);
    unsigned int n = ByteVector_GetSize(encryptedBytes);
    unsigned char checksum = (unsigned char)Checksum_SumBytes8(CourseModelByteBuffer_GetElement(encryptedBytes, 0), n);
    PasswordByteBuffer_Append(encryptedBytes, &checksum);
    unsigned int result = Base64_Encode(encryptedBytes, encodedText);
    if (result != 1) {
        { eh5Cleanup15.release(); ByteVector_dtor(encryptedBytes, -1); }
        { eh5Cleanup14.release(); ByteVector_dtor(plainBytes, -1); }
        return 0;
    }
    { eh5Cleanup15.release(); ByteVector_dtor(encryptedBytes, -1); }
    { eh5Cleanup14.release(); ByteVector_dtor(plainBytes, -1); }
    return result == 1;
}

// Normalize a texture path, check that it exists, and replace the shared texture handle.
int TexturePreload_LoadSharedResource(void* self, const char* name) {
    if (((PreloadTextureView*)self)->sharedTexture != 0) {
        void* textureResource = ((PreloadTextureView*)self)->sharedTexture;
        ((VObj*)textureResource)->f1();
        ((PreloadTextureView*)self)->sharedTexture = 0;
    }
    // Temporary path string: [flags/inline chars, heap length, heap pointer].
    // Own it through normalization, existence lookup and resource creation.
    int normalizedPathWords[3];
    normalizedPathWords[0] = 0;
    normalizedPathWords[1] = 0;
    normalizedPathWords[2] = 0;
    unsigned int len = strlen(name);
    String_ReserveCapacity(normalizedPathWords, len);
    int pathAllocatorStorage;
    char pathAllocatorTag = *(char*)&pathAllocatorStorage;
    String_ReplaceRange(normalizedPathWords, 0, 0, name, name + len, &pathAllocatorTag);
    StringUnwindCleanup_u8028DFB0 stringCleanup0(normalizedPathWords);
    ResourcePath_NormalizeImageExtension(normalizedPathWords);
    if (CHashPath_Exists(g_resourceManager, ((unsigned int)normalizedPathWords[0] >> 31) == 0 ? (char*)normalizedPathWords + 1 : *(char**)&normalizedPathWords[2]) == 0) {
        if (((unsigned int)normalizedPathWords[0] >> 31) != 0) {
            { stringCleanup0.release(); appFreeUni__FPv(*(void**)&normalizedPathWords[2]); }
        }
        return 0;
    }
    void* textureResource = CreateSharedTextureResource__FPCci(((unsigned int)normalizedPathWords[0] >> 31) == 0 ? (char*)normalizedPathWords + 1 : *(char**)&normalizedPathWords[2], 0);
    ((PreloadTextureView*)self)->sharedTexture = textureResource;
    if (textureResource == 0) {
        if (((unsigned int)normalizedPathWords[0] >> 31) != 0) {
            { stringCleanup0.release(); appFreeUni__FPv(*(void**)&normalizedPathWords[2]); }
        }
        return 0;
    }
    if (((unsigned int)normalizedPathWords[0] >> 31) != 0) {
        { stringCleanup0.release(); appFreeUni__FPv(*(void**)&normalizedPathWords[2]); }
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Encode the input byte vector as padded Base64, replacing the destination string.
int Base64_Encode(void* self, void* outputString) {
    int groupByteCount;
    int outputIndex;
    int remainingBytes;
    unsigned int inputIndex;
    unsigned int* encodedText;
    void* inputBytes;
    unsigned int groupAllocatorByte;
    groupByteCount = 0;
    encodedText = (unsigned int*)outputString;
    inputBytes = self;
    // Phase 1: clear output while retaining its capacity/allocation. Small-string ABI:
    // word0 bit31=heap; inline length is byte0&0x7f and characters start at +1;
    // heap mode stores length in word1 and character pointer in word2. Alphabet uses
    // this SAME representation, not a plain C string. Output must not alias input storage.
    if ((*(unsigned int*)encodedText >> 31) == 0) {
        ((unsigned char*)encodedText)[1] = 0;
        ((char*)encodedText)[0] &= ~0x7F;
    } else {
        *(char*)*(char**)&encodedText[2] = 0;
        ((unsigned int*)encodedText)[1] = 0;
    }
    unsigned char groupBytes[4];
    unsigned char alphabetIndices[4];
    remainingBytes = ((Buf*)inputBytes)->size;
    inputIndex = 0;
    // Phase 2: accumulate 3 input bytes -> four 6-bit alphabet indices -> append 4 chars.
    // Re-read alphabet/output storage after every append because replacement can allocate.
    while (remainingBytes--) {
        groupBytes[groupByteCount] = (*(unsigned char**)inputBytes)[inputIndex];
        inputIndex++;
        groupByteCount++;
        if (groupByteCount == 3) {
            alphabetIndices[0] = (groupBytes[0] & 0xFC) >> 2;
            alphabetIndices[1] = ((groupBytes[0] & 0x3) << 4) + ((groupBytes[1] & 0xF0) >> 4);
            alphabetIndices[2] = ((groupBytes[1] & 0xF) << 2) + ((groupBytes[2] & 0xC0) >> 6);
            alphabetIndices[3] = groupBytes[2] & 0x3F;
            char encodedCharacter[4];
            char groupAllocatorTag[4];
            int groupAllocatorStorage;
            groupAllocatorByte = *(unsigned char*)&groupAllocatorStorage;
            for (outputIndex = 0; outputIndex < 4; outputIndex++) {
                unsigned char alphabetIndex = alphabetIndices[outputIndex];
                unsigned int alphabetFlags = *(unsigned int*)nstring_access::s_base64Alphabet;
                char* alphabetData = (alphabetFlags >> 31) == 0 ? nstring_access::s_base64Alphabet + 1 : *(char**)(nstring_access::s_base64Alphabet + 8);
                encodedCharacter[0] = (char)alphabetData[alphabetIndex];
                unsigned int outputLength = (*(unsigned int*)encodedText >> 31) == 0 ? (((unsigned char*)encodedText)[0] & 0x7F) : ((unsigned int*)encodedText)[1];
                groupAllocatorTag[0] = groupAllocatorByte;
                String_ReplaceRange(encodedText, outputLength, 0, encodedCharacter, encodedCharacter + 1, groupAllocatorTag);
            }
            groupByteCount = 0;
        }
    }
    // Phase 3: zero-fill a final partial group, emit count+1 symbols, then pad with =.
    // Allocator-tag arrays carry runtime allocation metadata; characters live separately.
    if (groupByteCount != 0) {
        int outputIndex;
        for (int k = groupByteCount; k < 3; k++) {
            groupBytes[k] = 0;
        }
        alphabetIndices[0] = (groupBytes[0] & 0xFC) >> 2;
        alphabetIndices[1] = ((groupBytes[0] & 0x3) << 4) + ((groupBytes[1] & 0xF0) >> 4);
        alphabetIndices[2] = ((groupBytes[1] & 0xF) << 2) + ((groupBytes[2] & 0xC0) >> 6);
        alphabetIndices[3] = groupBytes[2] & 0x3F;
        char tailCharacter[4];
        char tailAllocatorTag[4];
        int tailAllocatorStorage;
        unsigned int tailAllocatorByte = *(unsigned char*)&tailAllocatorStorage;
        for (outputIndex = 0; outputIndex < groupByteCount + 1; outputIndex++) {
            unsigned char alphabetIndex = alphabetIndices[outputIndex];
            unsigned int alphabetFlags = *(unsigned int*)nstring_access::s_base64Alphabet;
            char* alphabetData = (alphabetFlags >> 31) == 0 ? nstring_access::s_base64Alphabet + 1 : *(char**)(nstring_access::s_base64Alphabet + 8);
            tailCharacter[0] = (char)alphabetData[alphabetIndex];
            unsigned int outputLength = (*(unsigned int*)encodedText >> 31) == 0 ? (((unsigned char*)encodedText)[0] & 0x7F) : ((unsigned int*)encodedText)[1];
            tailAllocatorTag[0] = tailAllocatorByte;
            String_ReplaceRange(encodedText, outputLength, 0, tailCharacter, tailCharacter + 1, tailAllocatorTag);
        }
        char paddingCharacter[4];
        char paddingAllocatorTag[4];
        int paddingAllocatorStorage;
        unsigned int paddingAllocatorByte = *(unsigned char*)&paddingAllocatorStorage;
        while (groupByteCount++ < 3) {
            paddingCharacter[0] = '=';
            unsigned int outputLength = (*(unsigned int*)encodedText >> 31) == 0 ? (((unsigned char*)encodedText)[0] & 0x7F) : ((unsigned int*)encodedText)[1];
            paddingAllocatorTag[0] = paddingAllocatorByte;
            String_ReplaceRange(encodedText, outputLength, 0, paddingCharacter, paddingCharacter + 1, paddingAllocatorTag);
        }
    }
    return 1;
}
#pragma pop

// Return the sum of all input bytes modulo 256.
int Checksum_SumBytes8(const void* data, unsigned int len) {
    const unsigned char* p = (const unsigned char*)data;
    unsigned int sum = 0;
    unsigned int i = 0;
    for (i = 0; i < len; i++) {
        sum += p[i];
    }
    return (int)(unsigned char)sum;
}

} // extern "C"

// These five .ctors entries initialize the cached angles; preserve their original order.
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046242C", _ctors_u8028DFB0_0, InitUtilityFullTurnRadiansA);
STATIC_INIT_ENTRY(".CRT$XCU8046242C", _ctors_u8028DFB0_1, InitUtilityFullTurnRadiansB);
STATIC_INIT_ENTRY(".CRT$XCU8046242C", _ctors_u8028DFB0_2, InitTexturePreloadFullTurnRadiansA);
STATIC_INIT_ENTRY(".CRT$XCU8046242C", _ctors_u8028DFB0_3, InitTexturePreloadFullTurnRadiansB);
STATIC_INIT_ENTRY(".CRT$XCU8046242C", _ctors_u8028DFB0_4, InitTexturePreloadFullTurnRadiansC);
#pragma pop

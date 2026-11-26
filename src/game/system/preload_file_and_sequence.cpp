// Preload objects for files, sequences and sprays (CPreloadFile, CPreloadSeq, CPreloadSpr).
// Each one reads a script stream, opens the named resources and queues them to load.
// Also the puppet resource preloader and the container that owns a sequence's preloads.

#include <decomp/static_init.h>

#include "sdk_protos.h"
#include "classes/CPreloadFile.h"
#include "classes/CPreloadSeq.h"
#include "classes/CPreloadSpr.h"
#include "classes/CPreloadWPuppet.h"
#include "classes/CPreloadParent.h"

#define AsCPreloadSeq(p) ((CPreloadSeq*)(p))

// Offsets into the packed resource and spray-script string pools.
enum PreloadStringOffset {
    kPuppetVectorLengthError = 5,
    kSprayExtension = 85,
    kExtensionCompareError = 90,
    kSequenceExtension = 171,
    kArchiveExtension = 176,
    kScriptLineFormat = 6,
    kLabelDelimiter = 9,
    kLineDelimiters = 11,
    kAssignmentDelimiter = 14,
    kSprayKeyword = 16,
    kArgumentDelimiter = 22,
    kArgumentTrimCharacters = 24,
    kScriptVectorLengthError = 30,
};
enum PreloadCapacity {
    kMaxResourcePointers = 0x3FFFFFFF,
    kThreeFifthsGrowthLimit = 0x15555555,
    kHalfGrowthLimit = 0x2AAAAAAA,
};

extern "C" {
extern float kFloat_6_2831855;
extern float s_filePreloadTwoPiStorage;
extern float s_sequencePreloadTwoPiStorage;
extern float s_sprayPreloadTwoPiStorage;
extern int g_resourceManager;
extern char __vt__12CPreloadFile[];
int OwnedObjectList_DestroyAll(void*);
void* Stream_VScanFormat(void*, void*, void*);
void __dt__14CPreloadParentFv(void*, int);
void __dl__FPv(void*);
extern char __vt__11CPreloadSeq[];
extern char __vt__11CPreloadSpr[];
extern char fxSprayScriptTokens[];
extern char puppetChunkResourceStrings[];
void* memcpy(void*, const void*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void Resource_Release(void*);
void appFreeUni__FPv(void*);
char* String_TrimCharacterSet(void*, char*);
int FxSprayScript_ParseEntry(void*, const char*, unsigned char*);
unsigned int String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, char*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12out_of_rangeFv();
void __dt__Q23std12length_errorFv();
void* memset(void*, int, unsigned int);
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct PreloadReadStream {
    virtual void s0();
    virtual void s1();
    virtual void* method(void*, void*);
};

struct PreloadFileReadView {
    virtual void slot0();
    virtual void slot1();
    virtual int valid();
    char filenameStorage[0x0C];
    PreloadReadStream* iface;
};

struct PreloadFileOpenView {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual int query(void*);
    char filenameStorage[0x0C];
    int iface;
};

struct SprayScriptStream {
    virtual void method(int);
};

struct SequenceScriptStream {
    virtual void method(int);
};

struct PreloadScriptStream {
    virtual void method(int);
};

struct SequencePreloadOpenView {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual int method(void*);
};

// Scalar view preserves the original integer stream handle operations.
struct PreloadStreamWordView {
    void** vtable;
    int filenameFlags;
    int filenameLength;
    int filenameBuffer;
    int streamHandle;
};

struct PreloadFileStorage {
    void* vtable;
    int filenameFlags;
    int filenameLength;
    int filenameBuffer;
    int stream;
};

typedef char PreloadFileStorage_stream_check[((int)&((PreloadFileStorage*)0)->stream == 0x10) ? 1 : -1];
typedef char PreloadStreamWord_stream_check[((int)&((PreloadStreamWordView*)0)->streamHandle == 0x10) ? 1 : -1];

struct PreloadStreamPositionView {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void* method();
};
// Stream vtable contracts established by these wrappers, not inferred API names:
// slot 2 forwards two opaque read arguments after the file preloader validity test;
// slot 6 forwards an opaque position/status word, absent stream -> 0;
// slot 7 forwards a predicate, absent stream -> 1. EOF polarity/position units
// require the concrete stream implementation and are unresolved in this unit.
// The sequence's *source wrapper* slot 2 is a DIFFERENT interface: file-complete.
// Script streams are owned and destroyed via slot 0(1); file resources release
// via Resource_Release. GetLoadedData returns borrowed bytes, never ownership.

struct PreloadPositionView {
    char baseStorage[0x10];
    PreloadStreamPositionView* iface;
};

struct PreloadStatusView {
    void** vtable;
    char filenameStorage[0x0C];
    void* iface;
};

struct PreloadStreamSlot7View {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual int method();
};

struct PreloadStreamReadyView {
    virtual void s0();
    virtual void s1();
    virtual int method();
};

// Forward the read interface's two opaque arguments; buffer/count semantics are not established here.
static inline void* ForwardPreloadRead(PreloadReadStream* stream, void* readArgument1, void* readArgument2) {
    return stream->method(readArgument1, readArgument2);
}

// Return stream slot 6 unchanged; its position/status units remain unresolved.
static inline void* QueryPreloadStreamWord(PreloadStreamPositionView* stream) {
    return stream->method();
}

// Return stream slot 7 unchanged; callers must not assume an EOF polarity.
static inline int QueryPreloadStreamPredicate(PreloadStreamSlot7View* stream) {
    return stream->method();
}

// Query completion on the SOURCE wrapper interface, distinct from a read-stream method.
static inline int QuerySequenceSourceCompletion(PreloadStreamReadyView* source) {
    return source->method();
}

struct SequencePreloadStorage {
    void* vtable;
    char filenameStorage[0x0C];
    void* iface;
};

// 0x802726E4: tail-call with the caller's owner pointer; the caller at 0x8026FFA8 tests the result at 0x80270024.
// Destroy every resource owned by the sequence preload container.
int SequencePreload_DestroyOwnedObjects(void* arg) {
    return OwnedObjectList_DestroyAll(arg);
}
}

namespace app {
template <class T> struct app_allocator {};
}
namespace std {
template <class T> struct char_traits {};
template <class T, class Traits, class Allocator> struct LocalBasicString_u80271734 {
    union {
        unsigned int flags;
        unsigned char labelText;
    };
    unsigned int size;
    char* data;
    // Free only the heap representation of this local path copy.
    ~LocalBasicString_u80271734() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
};
}

extern "C" {
typedef std::LocalBasicString_u80271734<char, std::char_traits<char>, app::app_allocator<char> > PreloadPathString;

struct FnThrowObj {
    void* vt;
    int ref1;
    int ref2;
    void* buf;
    // Take the exception message buffer into its shared ownership block.
    FnThrowObj(void* buffer) {
        ref1 = 1;
        ref2 = 1;
        vt = (void*)s_sharedCharDeleterVtable;
        buf = buffer;
    }
};

struct FnVectorGuard {
    void** data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* capacityAddress;
    unsigned int offset;
    // Start with no temporary array while retaining the original capacity address.
    FnVectorGuard(unsigned int* a) {
        data = 0;
        size = 0;
        capacity = 0;
        capacityAddress = a;
        offset = 0;
    }
    // Free the current scratch array, never the pointed-to resource objects.
    ~FnVectorGuard() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
}
namespace std {
namespace detail {
struct CharArrayGuard_u80271734 {
    char* ptr;
    char* allocator;
    // Arm cleanup for an exception-message character array.
    CharArrayGuard_u80271734(char* p, char* a) : ptr(p), allocator(a) {}
    // Free the array unless its ownership was transferred.
    ~CharArrayGuard_u80271734() {
        if (ptr)
            __dla__FPv(ptr);
    }
};
}
}
extern "C" {
typedef std::detail::CharArrayGuard_u80271734 FnCharGuard;

struct AllocatorScratchByte {
    char c;
};
}

namespace std {
class exception {
public:
    // Construct the stateless base exception.
    exception() {}
    virtual ~exception();
    virtual const char* what() const;
};
class logic_error : public exception {
    char* message;
    FnThrowObj* ref;

public:
    // Copy the borrowed error text and create its shared owner.
    logic_error(const char* msg) {
        AllocatorScratchByte alloc;
        alloc.c = 0;
        unsigned int n = strlen(msg) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        message = buffer;
        {
            FnCharGuard guard(buffer, &alloc.c);
            ref = new FnThrowObj(buffer);
            guard.ptr = 0;
        }
        strcpy(message, msg);
    }
    virtual ~logic_error();
    virtual const char* what() const;
};
class out_of_range : public logic_error {
public:
    // Construct the string-range error through logic_error.
    out_of_range(const char* msg) : logic_error(msg) {}
    virtual ~out_of_range();
};
class length_error : public logic_error {
public:
    // Construct the vector-capacity error through logic_error.
    length_error(const char* msg) : logic_error(msg) {}
    virtual ~length_error();
};
}

extern "C" {

// Return the smaller preload-string length by reference.
inline const unsigned int& PreloadMin(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}

// Return the larger preload capacity request by reference.
inline const unsigned int& PreloadMax(const unsigned int& growthCandidate, const unsigned int& requiredSlots) {
    return growthCandidate < requiredSlots ? requiredSlots : growthCandidate;
}

// Compare equal-prefix preload strings by length.
inline int ComparePreloadLengths(unsigned int leftLength, unsigned int rightLength) {
    if (leftLength < rightLength)
        return -1;
    if (leftLength > rightLength)
        return 1;
    return 0;
}

// Scan backward from the clamped start index; return unsigned -1 if no dot exists.
inline unsigned int FindLastExtensionDot(unsigned int size, unsigned int idx, char* data) {
    if (size != 0) {
        unsigned int cand = size - 1;
        if (cand < idx)
            idx = cand;
        char* p = data + idx;
        for (;;) {
            if (*p == '.')
                return (unsigned int)(p - data);
            if (p > data)
                --p;
            else
                break;
        }
    }
    return (unsigned int)-1;
}

// Compare a path suffix with a packed-pool token (all caller state explicit).
// The dot index includes '.', is checked against decoded byte length, and limits
// comparison to min(remaining,4) bytes, then token length breaks equal prefixes.
// Retain this expansion: the equivalent inline function changes CodeWarrior's
// exception scratch frame and code bytes. PreloadMin/ComparePreloadLengths below
// remain ordinary helpers; this macro owns the throw at each dispatch call site.
// In plain C each use is:
//   ext = <string table>[extensionOffset]; pathBytes/pathLength = inline (heapFlag==0, length in
//   the low 7 bits of the first byte, text from byte 1) or heap (data/size) string;
//   if (dotIndex > pathLength) throw std::out_of_range;
//   n = min(pathLength - dotIndex, 4);   (so only the first 4 bytes from the dot are compared)
//   comparison = memcmp(pathBytes + dotIndex, ext, min(n, strlen(ext)));
//   if (comparison == 0) comparison = compare_lengths(n, strlen(ext));
// i.e. a suffix matches a token when the compared bytes are equal and the lengths agree.
#define COMPARE_PRELOAD_EXTENSION(path, dotIndex, extensionOffset, heapFlag, comparison)                       \
    do {                                                                                                       \
        const char* extension = (char*)puppetChunkResourceStrings + (extensionOffset);                         \
        unsigned int comparedLength;                                                                           \
        unsigned int remainingLength;                                                                          \
        unsigned int extensionLength;                                                                          \
        unsigned int maximumExtensionLength;                                                                   \
        extensionLength = strlen(extension);                                                                   \
        maximumExtensionLength = 4;                                                                            \
        char* pathBytes;                                                                                       \
        unsigned int pathLength;                                                                               \
        if ((heapFlag) == 0) {                                                                                 \
            pathBytes = (char*)&(path) + 1;                                                                    \
            pathLength = (path).labelText & 0x7f;                                                              \
        } else {                                                                                               \
            pathBytes = (path).data;                                                                           \
            pathLength = (path).size;                                                                          \
        }                                                                                                      \
        if ((dotIndex) > pathLength) {                                                                         \
            throw std::out_of_range((char*)puppetChunkResourceStrings + kExtensionCompareError);               \
        }                                                                                                      \
        remainingLength = pathLength - (dotIndex);                                                             \
        comparedLength = PreloadMin(remainingLength, maximumExtensionLength);                                  \
        (comparison) = memcmp(pathBytes + (dotIndex), extension, PreloadMin(comparedLength, extensionLength)); \
        if ((comparison) == 0) {                                                                               \
            (comparison) = ComparePreloadLengths(comparedLength, extensionLength);                             \
        }                                                                                                      \
    } while (0)

struct PuppetResourcePreloadSet {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual int createResource(char*);
    virtual int createSpray(char*);
    virtual int createSequence(char*);
    void** data;
    unsigned int count;
    unsigned int capacity;
};

#pragma push
#pragma use_lmw_stmw on
// Dispatch a puppet resource by extension and retain a successfully created preload item.
int CPreloadWPuppet::QueueExternalResource(const char* input) {
    // input is a borrowed NUL-terminated path, copied for dispatch. Return 0 only
    // when there is no dot; return 1 even if no resource was created/retained.
    // Phase 1: decode the ABI string and locate the LAST dot (including its '.').
    PreloadPathString text;
    text.flags = 0;
    text.size = 0;
    text.data = 0;
    unsigned int len = strlen(input);
    String_ReserveCapacity(&text, len);
    AllocatorScratchByte allocator;
    AllocatorScratchByte empty = allocator;
    String_ReplaceRange(&text, 0, 0, input, input + len, &empty.c);
    unsigned int large = text.flags >> 31;
    unsigned int dot;
    unsigned int idx = (unsigned int)-1;
    char* data;
    unsigned int size;
    if (large == 0) {
        data = (char*)&text + 1;
        size = *(unsigned char*)&text & 0x7f;
    } else {
        data = text.data;
        size = text.size;
    }
    dot = FindLastExtensionDot(size, idx, data);
    if (dot == (unsigned int)-1) {
        return 0;
    }
    // Phase 2: spray/sequence extension tokens -> corresponding factory; archive
    // extension token -> intentionally skip;
    // other dotted suffixes -> generic resource. Comparison is case-sensitive
    // and covers at most four bytes from the dot, including the dot itself.
    PuppetResourcePreloadSet* self = (PuppetResourcePreloadSet*)this;
    int result = 0;
    int c;
    COMPARE_PRELOAD_EXTENSION(text, dot, kSprayExtension, large, c);
    if (c == 0) {
        result = self->createSpray((text.flags >> 31) == 0 ? (char*)&text + 1 : text.data);
    } else {
        COMPARE_PRELOAD_EXTENSION(text, dot, kSequenceExtension, text.flags >> 31, c);
        if (c == 0) {
            result = self->createSequence((text.flags >> 31) == 0 ? (char*)&text + 1 : text.data);
        } else {
            COMPARE_PRELOAD_EXTENSION(text, dot, kArchiveExtension, text.flags >> 31, c);
            if (c != 0)
                result = self->createResource((text.flags >> 31) == 0 ? (char*)&text + 1 : text.data);
        }
    }
    // Phase 3 is a push_back of `result` into self->data (a std::vector<void*>-style growth,
    // spelled out because the original code inlined it; see FxSprayScript_ParseEntry for the
    // identical copy). Fast path appends into capacity. Slow
    // path checks count overflow, reserves, copies old pointer slots, clears
    // their old copies, and swaps storage. Items themselves are never duplicated.
    if (result != 0) {
        unsigned int count = self->count;
        unsigned int cap = self->capacity;
        if (count < cap) {
            count++;
            self->count = count;
            self->data[count - 1] = (void*)result;
        } else {
            unsigned int max = kMaxResourcePointers;
            if (max - cap < 1) {
                throw std::length_error((char*)puppetChunkResourceStrings + kPuppetVectorLengthError);
            }
            unsigned int growthThree;
            unsigned int growthHalf;
            unsigned int n = self->count + 1 - self->capacity;
            FnVectorGuard storage(&self->capacity);
            cap = self->capacity;
            if (n > max - cap) {
                throw std::length_error((char*)puppetChunkResourceStrings + kPuppetVectorLengthError);
            }
            if (cap < kThreeFifthsGrowthLimit) {
                growthThree = (cap + 1) * 3 / 5;
                cap += PreloadMax(growthThree, n);
            } else if (cap < kHalfGrowthLimit) {
                growthHalf = (cap + 1) >> 1;
                cap += PreloadMax(growthHalf, n);
            } else {
                cap = max;
            }
            unsigned int newcap = cap;
            if (newcap > max) {
                throw std::length_error((char*)puppetChunkResourceStrings + kPuppetVectorLengthError);
            }
            // Guard owns only replacement-array storage until the final swap;
            // the container retains item ownership after commit. The guard then
            // frees the OLD array, not the pointed-to resources.
            void** nb = (void**)appMallocDummy__FUl(newcap * 4);
            unsigned int oldsize = self->count;
            storage.offset = oldsize;
            storage.data = nb;
            (storage.data + storage.offset)[storage.size++] = (void*)result;
            void** ob = self->data;
            void** end = ob + self->count;
            storage.capacity = newcap;
            int moved = ((char*)end - (char*)ob) / 4;
            storage.offset = oldsize - moved;
            memcpy(storage.data + storage.offset, ob, moved * 4);
            memset(ob, 0, moved * 4);
            storage.size += moved;
            void** olddata = self->data;
            void** newdata = storage.data;
            unsigned int oldcap = self->capacity;
            self->count = 0;
            storage.data = olddata;
            self->capacity = storage.capacity;
            storage.capacity = oldcap;
            self->data = newdata;
            self->count = storage.size;
            storage.size = 0;
        }
    }
    return 1;
}

#pragma pop

#pragma push
#pragma opt_common_subs off
// Release the file stream and any heap-backed filename, then optionally delete the wrapper.
CPreloadFile::~CPreloadFile() {
    void* iface = fileStream;
    *(void**)this = __vt__12CPreloadFile;
    if (iface != 0) {
        if (iface != 0) {
            Resource_Release(iface);
            fileStream = 0;
        }
        fileStream = 0;
    }
    if (&filenameFlags != 0) { // CPreloadFile::~CPreloadFile: string subobject +4
        if (((filenameFlags >> 31) != 0)) {
            appFreeUni__FPv(filenameBuffer);
        }
    }
}
#pragma pop

// Initialize the file-preloader angle constant.
void CPreloadFile_InitTwoPiConstant() {
    s_filePreloadTwoPiStorage = kFloat_6_2831855;
}

// Initialize the sequence-preloader angle constant.
void CPreloadSeq_InitTwoPiConstant() {
    s_sequencePreloadTwoPiStorage = kFloat_6_2831855;
}

// Query stream virtual slot 7; an absent stream reports true.
int CPreloadFile::QueryStreamSlot7() {
    PreloadStatusView* self = (PreloadStatusView*)this;
    PreloadStreamSlot7View* iface = (PreloadStreamSlot7View*)self->iface;
    if (iface != 0) {
        return QueryPreloadStreamPredicate(iface);
    }
    return 1;
}

// Initialize a sequence preloader with an empty owned script stream.
// Initialize a sequence preloader with an empty owned script stream.
CPreloadSeq* CPreloadSeq::Init() {
    ((CPreloadParent*)this)->__ct();
    scriptStream = 0;
    vtbl = __vt__11CPreloadSeq;
    return this;
}

// Query source-file preloader completion through virtual slot 2; an absent source reports true.
// The sequence factories supply CPreloadFileSync/Async, whose slot 2 reports file-load completion.
int SequencePreload_QueryFileCompletion(PreloadStatusView* self) {
    PreloadStreamReadyView* iface = (PreloadStreamReadyView*)self->iface;
    if (iface != 0) {
        return QuerySequenceSourceCompletion(iface);
    }
    return 1;
}

// Initialize the spray-preloader angle constant.
void CPreloadSpr_InitTwoPiConstant() {
    s_sprayPreloadTwoPiStorage = kFloat_6_2831855;
}

// Return the loaded stream data pointer after the preload validity check succeeds.
void* CPreloadFile::GetLoadedData() {
    PreloadFileReadView* self = (PreloadFileReadView*)this;
    int valid = self->valid();
    if (valid == 0) {
        return 0;
    }
    PreloadReadStream* iface = self->iface;
    if (iface != 0) {
        return (void*)((void**)iface)[1];
    }
    return 0;
}

// Replace the current stream by opening the requested resource through the virtual loader.
int CPreloadFile::OpenResource(void* arg) {
    if (arg == 0)
        return 0;
    int result;
    if (((PreloadFileOpenView*)this)->iface == 0) {
        result = 1;
    } else {
        if (((PreloadFileOpenView*)this)->iface != 0) {
            Resource_Release((void*)((PreloadFileOpenView*)this)->iface);
            ((PreloadFileOpenView*)this)->iface = 0;
        }
        ((PreloadFileOpenView*)this)->iface = 0;
        result = 1;
    }
    if (result == 0)
        return 0;
    if (g_resourceManager == 0)
        return 0;
    ((PreloadFileOpenView*)this)->iface = ((PreloadFileOpenView*)this)->query(arg);
    return ((PreloadFileOpenView*)this)->iface != 0;
}

// Forward a read request to the stream after checking preload validity.
void* CPreloadFile::Read(void* opaqueReadArgument1, void* opaqueReadArgument2) {
    int valid = ((PreloadFileReadView*)this)->valid();
    if (valid == 0) {
        return 0;
    }
    PreloadReadStream* iface = (PreloadReadStream*)((PreloadFileReadView*)this)->iface;
    if (iface != 0) {
        return ForwardPreloadRead(iface, opaqueReadArgument1, opaqueReadArgument2);
    }
    return 0;
}

// Initialize a spray preloader with an empty owned script stream.
CPreloadSpr::CPreloadSpr() {
    stream = 0;
    vtbl = __vt__11CPreloadSpr;
}

// Destroy the owned sequence script stream and the preload base.
void* CPreloadSeq::Destroy(int should_delete) {
    if (this != 0) {
        SequenceScriptStream* iface = (*(SequenceScriptStream**)&scriptStream);
        vtbl = __vt__11CPreloadSeq;
        if (iface != 0) {
            if (iface != 0) {
                if (iface != 0) {
                    iface->method(1);
                }
                scriptStream = 0;
            }
        }
        __dt__14CPreloadParentFv(this, 0);
        if (should_delete > 0)
            __dl__FPv(this);
    }
    return this;
}

// Scan script lines until a spray entry is queued or the stream is exhausted.
int CPreloadSeq::ProcessScriptEntries() {
    PreloadStreamWordView* self = (PreloadStreamWordView*)this;
    // 0 means one spray entry was queued (yield and resume next poll); 1 means
    // no source or scan exhaustion. This is a work-completion result, not success
    // of every parsed line. temp is a trusted 0x400-byte scan destination.
    if (self->streamHandle == 0)
        return 1;
    unsigned char match;
    char temp[0x400];
    // Scan/trim each line; changed is independent of ParseEntry's return value.
    while (((CPreloadFile*)self->streamHandle)->ScanFormat(fxSprayScriptTokens + kScriptLineFormat, temp)) {
        match = 0;
        char* name = String_TrimCharacterSet(temp, fxSprayScriptTokens);
        FxSprayScript_ParseEntry(self, name, &match);
        if (match != 0)
            return 0;
    }
    return 1;
}

void cTokenV_Init(void*);
void cTokenV_SetInputBuffer(void*, const char*, unsigned int);
void cToken_ReadUntilDelimiter(void*, void*, const char*, int);
void cToken_dtor(void*, int);
char* String_TrimCharacterSet(void*, char*);
int CString_Compare(const char*, const char*);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
char* strcpy(char*, const char*);
void* __nw__FUl(unsigned int);
void __throw(const char*, void*, void*);
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
void __dt__Q23std12length_errorFv();

struct Fn27Token {
    char state[0x18];
    // Initialize an empty tokenizer over borrowed input bytes.
    Fn27Token() {
        cTokenV_Init(this);
    }
    // Destroy tokenizer state without deleting its stack storage.
    ~Fn27Token() {
        cToken_dtor(this, 0);
    }
};

struct SprayResourcePreloadSet {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void* createItem(const char*);
    void** data;
    unsigned int count;
    unsigned int capacity;
};

// Throw the preload resource vector's capacity error.
inline void Fn27Throw(const char* message) {
    throw std::length_error(message);
}

#pragma push
#pragma use_lmw_stmw on
// Parse a spray assignment and append the created resource to the owned-item vector.
int FxSprayScript_ParseEntry(void* object, const char* input, unsigned char* changed) {
    // changed is always initialized to 0 and becomes 1 only after append commits.
    // Return 0 for empty/comment input, empty/comment statement or factory failure;
    // 1 also includes non-spray statements that append nothing. Caller must use
    // changed to detect new work. Tokenizer fills fixed trusted-size buffers.
    // Phase 1: split label/statement, strip comments/whitespace, then split '='.
    SprayResourcePreloadSet* self = (SprayResourcePreloadSet*)object;
    *changed = 0;
    if (*input == 0 || *input == ';') {
        return 0;
    }
    Fn27Token text;
    char labelText[0x104];
    char statementText[0x104];
    char assignmentName[0x104];
    char resourcePath[0x104];
    cTokenV_SetInputBuffer(&text, input, strlen(input));
    cToken_ReadUntilDelimiter(&text, labelText, fxSprayScriptTokens + kLabelDelimiter, 1);
    cToken_ReadUntilDelimiter(&text, statementText, fxSprayScriptTokens + kLineDelimiters, 2);
    String_TrimCharacterSet(statementText, fxSprayScriptTokens);
    if (*statementText == ';')
        return 0;
    if (*statementText == 0)
        return 0;
    cTokenV_SetInputBuffer(&text, statementText, strlen(statementText));
    cToken_ReadUntilDelimiter(&text, assignmentName, fxSprayScriptTokens + kAssignmentDelimiter, 1);
    String_TrimCharacterSet(assignmentName, fxSprayScriptTokens);
    // Phase 2: recognize the spray assignment and construct its referenced item.
    if (CString_Compare(assignmentName, fxSprayScriptTokens + kSprayKeyword) == 0) {
        cToken_ReadUntilDelimiter(&text, resourcePath, fxSprayScriptTokens + kArgumentDelimiter, 1);
        String_TrimCharacterSet(resourcePath, fxSprayScriptTokens + kArgumentTrimCharacters);
        void* item = self->createItem(resourcePath);
        if (item == 0)
            return 0;
        {
            // Phase 3: append or reserve/copy/swap exactly as the puppet queue.
            // The array guard frees storage only; created items transfer to self.
            unsigned int count = self->count;
            unsigned int capacity = self->capacity;
            if (count < capacity) {
                self->data[++self->count - 1] = item;
            } else {
                unsigned int maximumCount = kMaxResourcePointers;
                if (maximumCount - capacity < 1) {
                    Fn27Throw(fxSprayScriptTokens + kScriptVectorLengthError);
                }
                unsigned int growthThree;
                unsigned int growthHalf;
                unsigned int requiredAdditionalSlots = self->count + 1 - self->capacity;
                FnVectorGuard storage(&self->capacity);
                capacity = self->capacity;
                if (requiredAdditionalSlots > maximumCount - capacity) {
                    Fn27Throw(fxSprayScriptTokens + kScriptVectorLengthError);
                }
                if (capacity < kThreeFifthsGrowthLimit) {
                    growthThree = (capacity + 1) * 3 / 5;
                    capacity += PreloadMax(growthThree, requiredAdditionalSlots);
                } else if (capacity < kHalfGrowthLimit) {
                    growthHalf = (capacity + 1) >> 1;
                    capacity += PreloadMax(growthHalf, requiredAdditionalSlots);
                } else {
                    capacity = maximumCount;
                }
                unsigned int newCapacity = capacity;
                if (newCapacity > maximumCount) {
                    Fn27Throw(fxSprayScriptTokens + kScriptVectorLengthError);
                }
                void** replacementArray = (void**)appMallocDummy__FUl(newCapacity * 4);
                unsigned int oldCount = self->count;
                storage.offset = oldCount;
                storage.data = replacementArray;
                (storage.data + storage.offset)[storage.size++] = item;
                void** oldArray = self->data;
                void** end = oldArray + self->count;
                storage.capacity = newCapacity;
                int moved = ((char*)end - (char*)oldArray) / 4;
                storage.offset = oldCount - moved;
                // Copy borrowed slots into replacement storage before clearing
                // the old slots; swap ownership only after the move is complete.
                memcpy(storage.data + storage.offset, oldArray, moved * 4);
                memset(oldArray, 0, moved * 4);
                storage.size += moved;
                void** olddata = self->data;
                void** newdata = storage.data;
                unsigned int oldcap = self->capacity;
                self->count = 0;
                storage.data = olddata;
                self->capacity = storage.capacity;
                storage.capacity = oldcap;
                self->data = newdata;
                self->count = storage.size;
                storage.size = 0;
            }
            *changed = 1;
        }
    }
    return 1;
}
#pragma pop

// Close the prior script stream and replace it through the virtual stream factory.
int CPreloadSeq::OpenStreamFromPath(void* arg1) {
    if (((PreloadStreamWordView*)this)->streamHandle != 0) {
        if (((PreloadStreamWordView*)this)->streamHandle != 0) {
            if (((PreloadStreamWordView*)this)->streamHandle != 0) {
                ((PreloadScriptStream*)((PreloadStreamWordView*)this)->streamHandle)->method(1);
            }
            ((PreloadStreamWordView*)this)->streamHandle = 0;
        }
    }
    int result = ((SequencePreloadOpenView*)this)->method(arg1);
    ((PreloadStreamWordView*)this)->streamHandle = result;
    return result != 0;
}

// Forward formatted scanning and its varargs to the open preload stream.
void* CPreloadFile::ScanFormat(void* fmt, ...) {
    PreloadStreamWordView* self = (PreloadStreamWordView*)this;
    if (self->streamHandle == 0)
        return 0;
    struct {
        unsigned int a;
        void* b;
        void* c;
    } ap = {0, 0, 0};
    __va_setup(&ap);
    return Stream_VScanFormat((void*)self->streamHandle, fmt, &ap);
}

// Destroy the owned spray script stream and the preload base.
CPreloadSpr::~CPreloadSpr() {
    vtbl = __vt__11CPreloadSpr;
    if (stream != 0) {
        if (stream != 0) {
            SprayScriptStream* old = (SprayScriptStream*)stream;
            old->method(1);
        }
        stream = 0;
    }
}

// Initialize the filename string and clear the stream handle.
CPreloadFile::CPreloadFile() {
    PreloadFileStorage* self = (PreloadFileStorage*)this;
    self->filenameFlags = 0;
    self->vtable = &__vt__12CPreloadFile;
    self->filenameLength = 0;
    self->filenameBuffer = 0;
    self->stream = 0;
}

// Return the stream position/status slot, or zero when no stream is open.
void* CPreloadFile::QueryStreamSlot6() {
    PreloadPositionView* self = (PreloadPositionView*)this;
    PreloadStreamPositionView* iface = self->iface;
    if (iface != 0) {
        return QueryPreloadStreamWord(iface);
    }
    return 0;
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462370", _ctors_u80271734_0, CPreloadFile_InitTwoPiConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462370", _ctors_u80271734_1, CPreloadSeq_InitTwoPiConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462370", _ctors_u80271734_2, CPreloadSpr_InitTwoPiConstant);
#pragma pop

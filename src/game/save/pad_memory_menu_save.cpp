// Controller-memory save menu and synchronous preloads: the save and title checks, the save state
// machine, and the preloaders for files, sprites, textures, puppets and script directives.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "MSL/error_message.h"
// Wii Remote save menu and synchronous puppet-resource preloading.
// Mod lookup:
// - save confirmation/messages: PadSaveState below, memorySaveDialogMessages
//   (six byte extents of UTF-16 message text), CPadMemoryMenuSave::AdvanceSaveStateMachine.
// - retry policy: kPadSaveMaxRetries=3 means three EXTRA writes on verification
//   mismatch only (four attempts total); failure/disconnection do not retry.
// - serialized logical record: kPadSaveRecordBytes=0x47C; InitSaveRecordAndFade
//   owns a snapshot, GetSaveRecord borrows it or delegates to the common summary.
//   SaveForm replication/digest and device alignment are separate in pad_memory_menu_load.cpp.
// - preload grammar: CPreloadSpr::ParseResourceDirective, PreloadScriptStrings;
//   pointer-vector growth changes storage, not loaded-resource lifetimes.
// Base UI/dialog/guard cleanup: CFileIoMenu in file_io_menu_save_load_base.cpp. Device access and
// verification: pad_memory_menu_load.cpp; access state machines/buffers: file_io_access_read_write.cpp and
// file_io_flash_and_write_access.cpp; singleton device callbacks: file_io_pad_and_flash_io.cpp. See ownership map
// before the save update routine. There is no device callback owned by this menu.

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CPadMemoryMenuSave.h"
#include "classes/CPadMemoryMenu.h"
#include "classes/CPreloadWPuppet.h"
#include "classes/CFileIoMenu.h"
#include "classes/CPreloadSpr.h"
#include "classes/CPreloadWPuppetSync.h"
#include "classes/CPreloadSeqSync.h"
#include "classes/CPreloadFile.h"
#include "classes/CPreloadFileSync.h"
#include "classes/CPreloadTextureSync.h"
#include "classes/CPreloadSprSync.h"

#define AsCPadMemoryMenu(p) ((CPadMemoryMenu*)(p))

#define AsCPadMemoryMenuSave(p) ((CPadMemoryMenuSave*)(p))

// Controller-memory save flow; state numbers are part of the existing menu protocol.
enum PadSaveState {
    kPadSaveCheckScreen = 0,
    kPadSaveCheckExistingTitle = 1,
    kPadSaveOpenOverwritePrompt = 2,
    kPadSaveWaitOverwritePrompt = 3,
    kPadSaveWaitRead = 4,
    kPadSaveWaitSavePrompt = 5,
    kPadSaveStartWrite = 6,
    kPadSaveWaitWrite = 7,
    kPadSaveWaitDisconnectMessage = 8,
    kPadSaveWaitVerificationRead = 9,
    kPadSaveWaitSuccessMessage = 10,
    kPadSaveWaitFailureMessage = 11,
    kPadSaveFinish = 12,
    kPadSaveComplete = 13
};
enum {
    kPadSaveMaxRetries = 3,
    kPadSaveRecordBytes = 0x47C,
    kPreloadTokenCount = 16
};
enum {
    kPreloadTailResourceArgument = 6,
    kPreloadTokenBytes = 0x104,
    kPreloadMaxPointers = 0x3fffffffU
};

// Save protocol: check screen -> query title -> read matching title or confirm
// overwrite -> write -> read back -> verify. A mismatch permits three additional
// writes; device/access errors go directly to failure dialogs. State 12 finishes
// the base menu, 13 is terminal. Update returning 1 means the tick succeeded,
// not that saving completed; IsSaveComplete tests state 13. Returns 0 propagate a base,
// dialog, I/O-creation, or finish-hook failure. Pending I/O leaves the state intact.

extern "C" {

int OwnedObjectList_DestroyAll(void*);
void SharedContext_SetCurrent(void*);
void __ct__11CFileIoMenuFv(void*, void*);
void __dt__15CPreloadWPuppetFv(void*, int);
void __dt__14CPadMemoryMenuFv(void*, int);
void* __nw__FUl(unsigned int);
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
void FadeState_SetAlternateDrawEnabled(void*, int);
void SaveRecordPrefix_ClearCtor(void*);
void SaveData_CopyRecordPrefix47C(void*, void*);
extern char __vt__18CPadMemoryMenuSave[];
extern char s_fadeOverlay[];
extern char s_padSaveFadeOverlayDtorRegistration[];
extern signed char s_fadeOverlayInitialized;
void* String_TrimCharacterSet(void*, const char*);
void cTokenV_Init(void*);
void cTokenV_SetInputBuffer(void*, const char*, unsigned int);
void cToken_ReadUntilDelimiter(void*, char*, const char*, int);
int FxSprayScriptBuffer_EnsureTerminatorCapacity(void*);
void cToken_dtor(void*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
int CString_Compare(void*, const char*);
void String_ReserveCapacity(void*, unsigned int);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv(void*, int);
void __dt__Q23std9exceptionFv(void*, int);
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char s_sharedCharDeleterVtable[];
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
struct PreloadScriptStrings {
    char whitespace[5]; // 0x0
    char padding5[1];
    char scanLine[3];               // 0x6
    char equalsDelimiter[2];        // 0x9
    char commaDelimiter[2];         // 0xB
    char argumentTrimCharacters[6]; // 0xD
    char sourceKeyword[7];          // 0x13
    char wordDelimiters[3];         // 0x1A
    char petKeyword[5];             // 0x1D
    char tailKeyword[5];            // 0x22
    char soundKeyword[6];           // 0x27
    char vectorLengthError[20];     // 0x2D
};
extern PreloadScriptStrings puppetPreloadParserStrings;
extern char __vt__19CPreloadWPuppetSync[];
void __dl__FPv(void*);
extern char __vt__14CPadMemoryMenu[];
int PadMemory_QueryStoredTitle(void*, int*);
void WString_ReserveCapacity(void*, int);
void WString_ReplaceRange(void*, int, int, void*, void*, void*);
void appFreeUni__FPv(void*);
struct PadSaveDialogStrings {
    char confirmOverwrite[50]; // 0x0
    char confirmSave[48];      // 0x32
    char saving[46];           // 0x62
    char disconnected[54];     // 0x90
    char saveFailed[52];       // 0xC6
    char saveSucceeded[58];    // 0xFA
};
extern PadSaveDialogStrings memorySaveDialogMessages;
extern unsigned short* g_padSaveGameTitle;
extern float s_puppetSyncFullTurn;
extern float kFloat_6_2831855;
extern float s_padMemoryMenuFullTurn;
extern float s_padMemorySaveFullTurn;

struct PadSaveStateView {
    char base[0x44];
    unsigned int saveState;
};

struct OwnedPreloadResource {
    virtual void destroy(int);
};
struct SpritePreloadView {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void* f6(void*);
    char base[0x0c];
    OwnedPreloadResource* child;
};

struct Fn80273814Obj {
    virtual void f0();
    virtual void f1();
    virtual void destroy(int);
    virtual void f3();
    virtual int init(void*);
    char base[0x10];
    // Initialize the puppet-preloader base in existing object storage.
    Fn80273814Obj() {
        ((CPreloadWPuppet*)this)->__ct();
    }
};

struct PadSaveCompletionView {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void f4();
    virtual void f5();
    virtual void f6();
    virtual void f7();
    virtual void fX();
    virtual int updateSave();
    // Same receiver, virtual update slot (table +0x2C).
    char base[0x40];
    unsigned int saveState;
};

// Destroy the synchronous puppet preloader's base part.
CPreloadWPuppetSync::~CPreloadWPuppetSync() {
    __dt__15CPreloadWPuppetFv(this, 0);
}

// Replace the previously owned child with a newly created preload resource.
int CPreloadSpr::OpenNamedSourceFile(void* arg) {
    if (((SpritePreloadView*)this)->child != 0) {
        if (((SpritePreloadView*)this)->child != 0) {
            ((SpritePreloadView*)this)->child->destroy(1);
        }
        ((SpritePreloadView*)this)->child = 0;
    }
    ((SpritePreloadView*)this)->child = (OwnedPreloadResource*)((SpritePreloadView*)this)->f6(arg);
    if (((SpritePreloadView*)this)->child == 0)
        return 0;
    return 1;
}

struct Fn80273814Alloc {
    char data[0x14];
    // Construct the puppet base and install the synchronous-preloader vtable.
    Fn80273814Alloc() {
        ((CPreloadWPuppet*)this)->__ct();
        *(void**)this = &__vt__19CPreloadWPuppetSync;
    }
};
struct PreloadInitDispatch {
    virtual void destroy(int);
    virtual void f1();
    virtual void f2();
    virtual int init(void*);
};
// Allocate and initialize a synchronous puppet preloader; destroy it on failure.
Fn80273814Alloc* CPreloadWPuppetSync::Create(void* arg) {
    Fn80273814Alloc* obj = new Fn80273814Alloc;
    PreloadInitDispatch* vobj = (PreloadInitDispatch*)obj;
    if (obj == 0)
        return 0;
    if (!vobj->init(arg)) {
        if (obj != 0)
            vobj->destroy(1);
        return 0;
    }
    return obj;
}

// Scan script lines until a resource is loaded, or the input is exhausted.
// Returns 0 after one successful resource append, allowing work to span ticks;
// returns 1 for end-of-input or no attached file. Unknown directives are skipped.
int CPreloadSpr::ProcessScriptEntries() {
    if (((CPreloadWPuppet*)this)->file == 0)
        return 1;
    unsigned char found;
    char buffer[0x400];
    while (((CPreloadFile*)(((CPreloadWPuppet*)this)->file))->ScanFormat(puppetPreloadParserStrings.scanLine, buffer)) {
        found = 0;
        ParseResourceDirective(String_TrimCharacterSet(buffer, puppetPreloadParserStrings.whitespace), &found);
        if (found)
            return 0;
    }
    return 1;
}

struct DirectiveTokenizer {
    unsigned int flags, position, inputLength;
    char padding0C[4];
    char* tokenBuffer;
    char padding14[4];
    // Initialize the directive tokenizer over borrowed input.
    DirectiveTokenizer() {
        cTokenV_Init(this);
    }
    // Release tokenizer-owned buffers without deleting this stack wrapper.
    ~DirectiveTokenizer() {
        cToken_dtor(this, 0);
    }
};
struct ArgumentTokenizer {
    unsigned int flags, position, inputLength;
    char padding0C[4];
    char* tokenBuffer;
    char padding14[4];
    // Initialize the nested source-argument tokenizer.
    ArgumentTokenizer() {
        cTokenV_Init(this);
    }
    // Release tokenizer-owned buffers without deleting this stack wrapper.
    ~ArgumentTokenizer() {
        cToken_dtor(this, 0);
    }
};
struct PreloadedResourceList {
    void** begin;
    unsigned int size, capacity;
};
// This pointer vector owns loaded resources; growing it moves pointer words and
// zeroes the old slots without releasing resources. DestroyOwnedObjects performs
// resource destruction. Realloc guards own only vector storage, not its payloads.
struct PetNameString {
    unsigned int flag;
    unsigned int size;
    char* data;
    // Initialize an empty inline pet-name string.
    PetNameString() : flag(0), size(0), data(0) {}
    // Release pet-name characters only when heap-owned.
    ~PetNameString() {
        if (flag >> 31)
            appFreeUni__FPv(data);
    }
};
struct Fn80272FE4Tag {
    unsigned char byte;
    char pad[3];
};
struct ScriptPreloadDispatch {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void f4();
    virtual void* loadPet(void*);
    virtual void* loadSource(void*);
};

// Select the larger requested growth amount by reference.
inline const unsigned int& MaxPreloadCapacity(const unsigned int& a, const unsigned int& b) {
    return b < a ? a : b;
}

struct Fn80272FE4DeleteArray {
    char* data;
    void* tag;
    // Release temporary error-text characters until shared ownership is installed.
    ~Fn80272FE4DeleteArray() {
        if (data)
            __dla__FPv(data);
    }
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control Fn80272FE4Control;
struct Fn80272FE4ErrorString {
    char* message;
    Fn80272FE4Control* control;
    // Copy borrowed error text into shared owned character storage.
    Fn80272FE4ErrorString(const char* const& text) {
        Fn80272FE4Tag tag;
        tag.byte = 0;
        unsigned int n = strlen(text) + 1;
        message = (char*)__nwa__FUl((n << 1) - n);
        char* p = message;
        {
            Fn80272FE4DeleteArray guard;
            guard.data = p;
            guard.tag = &tag;
            control = new Fn80272FE4Control(p);
            guard.data = 0;
        }
        strcpy(message, text);
    }
};
} // extern "C"
namespace std {
class exception {
public:
    // Initialize the reconstructed exception base for this local instantiation.
    exception() {}
    virtual ~exception();
    virtual const char* what() const;
};
class logic_error : public exception {
public:
    Fn80272FE4ErrorString message;
    // Construct the logic-error message from borrowed text.
    logic_error(const char* text) : message(text) {}
    virtual ~logic_error();
    virtual const char* what() const;
};
class length_error : public logic_error {
public:
    // Construct the length-error message from borrowed text.
    length_error(const char* text) : logic_error(text) {}
    virtual ~length_error();
};
}
extern "C" {
// Reject pointer-vector growth that exceeds the allocation limit.
inline void ThrowPreloadLengthError() {
    throw std::length_error(puppetPreloadParserStrings.vectorLengthError);
}
// Find the first ASCII space in a small or heap-backed string.
inline int FindPetNameSpace(PetNameString* out) {
    unsigned char* begin;
    unsigned int len;
    if ((out->flag >> 31) == 0) {
        len = *(unsigned char*)out & 0x7f;
        begin = (unsigned char*)out + 1;
    } else {
        begin = (unsigned char*)out->data;
        len = out->size;
    }
    if (len != 0) {
        unsigned char* end = begin + len;
        for (unsigned char* p = begin; p < end; ++p)
            if (*p == ' ')
                return p - begin;
    }
    return -1;
}
// Choose the original pointer-vector growth amount without changing temporary lifetimes.
inline unsigned int RecommendPreloadCapacity(unsigned int cap, const unsigned int& need, unsigned int& grown60,
                                             unsigned int& grown50) {
    if (cap < 0x15555555u) {
        grown60 = (cap + 1) * 3 / 5;
        return cap + MaxPreloadCapacity(need, grown60);
    }
    if (cap < 0x2aaaaaaau) {
        grown50 = (cap + 1) >> 1;
        return cap + MaxPreloadCapacity(need, grown50);
    }
    return 0x3fffffffu;
}
struct ResourceListGrowth {
    void** begin;
    unsigned int size, capacity;
    unsigned int* oldCapacity;
    unsigned int offset;
    // Initialize empty growth storage and borrow the original-capacity pointer.
    ResourceListGrowth(unsigned int* c) : begin(0), size(0), capacity(0), oldCapacity(c), offset(0) {}
    // Free vector storage still owned after growth/exchange; resources stay borrowed.
    ~ResourceListGrowth() {
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};

#pragma use_lmw_stmw on
// Parse source/tail directives, resolve pet names, and append a successfully loaded resource.
// inputArg borrows a NUL-terminated line. Returns 0 for blank/comment lines, 1
// otherwise, including unknown directives and load failure. loadedResource is a
// required output byte: only 1 means a resource was appended and ownership passed
// to self. Grammar: key=first,...,sixteenth; source uses the first argument (the petKeyword token
// followed by a name selects loadPet); tail uses the seventh argument. sound loads nothing here.
// Exact source spelling: source=*pet NAME [ignored suffix] uses loadPet(NAME);
// other source values use loadSource(first argument). Leading ';' is a comment only in byte
// zero; callers trim whole lines. Keys are case-sensitive CString comparisons.
// Key and EVERY comma token must fit 0x104 bytes including NUL (<=259 payload
// bytes before trimming). cToken_ReadUntilDelimiter (token_parser_and_bubble.cpp) has no output
// capacity parameter/check: oversized input can overwrite stack storage; there
// is no truncation/error return. It preserves two-byte character pairs.
// Fewer than sixteen args become empty strings at exhausted input; extras are
// ignored. Missing '=' is not diagnosed: the entire line becomes the key, and
// empty arguments follow. Empty tail (seventh argument) loads nothing. source with empty first argument
// or failed token-buffer allocation has no safe contract (null may reach strcmp).
// '*pet' without an ASCII space skips loading; only text between first and second
// ASCII spaces is the pet name (tabs delimit the first token but not name slicing).
// Required self and output byte must not alias input/token storage. Loader failure
// gives return 1 with output 0 and no append. Blank/comment input returns 0 with
// output 0. Pointer growth overflow throws length_error; append allocation can
// throw. The loaded resource has no local destroy guard before append, so this
// path supplies no resource rollback guarantee on allocation failure. Existing
// list storage remains owned by self; temp guards free only pointer arrays.
int CPreloadSpr::ParseResourceDirective(void* inputArg, unsigned char* loadedResource) {
    // Phase 1: split and trim the directive and its fixed sixteen argument slots.
    unsigned int grown60, grown50, additionalCapacity;
    ScriptPreloadDispatch* loader = (ScriptPreloadDispatch*)this;
    PreloadedResourceList* resources = (PreloadedResourceList*)((char*)this + 4);
    const char* input = (const char*)inputArg;
    char firstCharacter = input[0];
    *loadedResource = 0;
    if (firstCharacter == 0 || firstCharacter == ';')
        return 0;
    DirectiveTokenizer source;
    char key[kPreloadTokenBytes];
    char arguments[kPreloadTokenCount][kPreloadTokenBytes];
    cTokenV_SetInputBuffer(&source, input, strlen(input));
    cToken_ReadUntilDelimiter(&source, key, puppetPreloadParserStrings.equalsDelimiter, 1);
    String_TrimCharacterSet(key, puppetPreloadParserStrings.whitespace);
    for (int i = 0; i < kPreloadTokenCount; ++i) {
        cToken_ReadUntilDelimiter(&source, arguments[i], puppetPreloadParserStrings.commaDelimiter, 1);
        String_TrimCharacterSet(arguments[i], puppetPreloadParserStrings.argumentTrimCharacters);
    }
    // Phase 2: resolve one resource. source requires a readable first token;
    // this existing path assumes tokenizer storage and valid directive contents.
    void* resource = 0;
    if (CString_Compare(key, puppetPreloadParserStrings.sourceKeyword) == 0) {
        ArgumentTokenizer line;
        cTokenV_SetInputBuffer(&line, arguments[0], strlen(arguments[0]));
        char* token;
        const char* delimiters = puppetPreloadParserStrings.wordDelimiters;
        if (FxSprayScriptBuffer_EnsureTerminatorCapacity(&line) == 0)
            token = 0;
        else {
            token = line.tokenBuffer;
            *token = 0;
            while ((*(int*)&line.position) < (*(int*)&line.inputLength)) {
                cToken_ReadUntilDelimiter(&line, token, delimiters, 2);
                if (*token != 0)
                    break;
            }
        }
        if (strcmp(token, puppetPreloadParserStrings.petKeyword) == 0) {
            PetNameString petName;
            unsigned int n = strlen(arguments[0]);
            String_ReserveCapacity(&petName, n);
            volatile Fn80272FE4Tag termA1;
            Fn80272FE4Tag termB1;
            termB1.byte = termA1.byte;
            String_ReplaceRange(&petName, 0, 0, arguments[0], arguments[0] + n, &termB1);
            int firstSpace = FindPetNameSpace(&petName);
            if ((unsigned int)(firstSpace + 0x10000) != 0xffff) {
                Fn80272FE4Tag termB2;
                volatile Fn80272FE4Tag termA2;
                termB2.byte = termA2.byte;
                String_ReplaceRange(&petName, 0, firstSpace + 1, 0, 0, &termB2);
                int secondSpace = FindPetNameSpace(&petName);
                if ((unsigned int)(secondSpace + 0x10000) != 0xffff) {
                    Fn80272FE4Tag termB3;
                    volatile Fn80272FE4Tag termA3;
                    termB3.byte = termA3.byte;
                    String_ReplaceRange(&petName, secondSpace, -1, 0, 0, &termB3);
                }
                char* begin;
                if ((petName.flag >> 31) == 0)
                    begin = (char*)&petName + 1;
                else
                    begin = petName.data;
                resource = loader->loadPet(begin);
            }
        } else {
            resource = loader->loadSource(arguments[0]);
        }
    } else if (CString_Compare(key, puppetPreloadParserStrings.tailKeyword) == 0) {
        if (strlen(arguments[kPreloadTailResourceArgument]) != 0)
            resource = loader->loadSource(arguments[kPreloadTailResourceArgument]);
    } else
        CString_Compare(key, puppetPreloadParserStrings.soundKeyword);
    // Phase 3: append directly or grow/copy/exchange storage. The new resource
    // becomes vector-owned only on this path; old pointer slots are relinquished.
    if (resource) {
        unsigned int size = resources->size;
        unsigned int cap = resources->capacity;
        if (size < cap) {
            unsigned int next = size + 1;
            void** data = resources->begin;
            resources->size = next;
            data[next - 1] = (void*)resource;
        } else {
            if (kPreloadMaxPointers - cap < 1)
                ThrowPreloadLengthError();
            size = resources->size;
            cap = resources->capacity;
            additionalCapacity = size + 1 - cap;
            ResourceListGrowth temp(&resources->capacity);
            cap = resources->capacity;
            if (additionalCapacity > kPreloadMaxPointers - cap)
                ThrowPreloadLengthError();
            unsigned int newCap = RecommendPreloadCapacity(cap, additionalCapacity, grown60, grown50);
            if (newCap > kPreloadMaxPointers)
                throw std::length_error(puppetPreloadParserStrings.vectorLengthError);
            temp.begin = (void**)appMallocDummy__FUl(newCap * 4);
            temp.offset = resources->size;
            (temp.begin + temp.offset)[temp.size] = (void*)resource;
            ++temp.size;
            void** oldBegin = resources->begin;
            void** end = oldBegin + resources->size;
            temp.capacity = newCap;
            int count = end - oldBegin;
            temp.offset = resources->size - count;
            memcpy(temp.begin + temp.offset, oldBegin, count * 4);
            memset(oldBegin, 0, count * 4);
            temp.size += count;
            resources->size = 0;
            unsigned int oldCap = resources->capacity;
            void** old = resources->begin;
            resources->capacity = temp.capacity;
            temp.capacity = oldCap;
            resources->begin = temp.begin;
            temp.begin = old;
            resources->size = temp.size;
            temp.size = 0;
        }
        *loadedResource = 1;
    }
    return 1;
}
#pragma use_lmw_stmw reset

// Invoke this menu's virtual update; if it succeeds, report state 13, otherwise
// report complete (1). Thus update failure also ends polling; 1 is not proof of a
// successful save. Required live receiver; no attached-object/null test occurs.
int CPadMemoryMenuSave::UpdateAndReportSaveComplete(PadSaveCompletionView* arg) {
    if (arg->updateSave())
        return arg->saveState == kPadSaveComplete;
    return 1;
}

#pragma dont_inline on
// Destroy the controller-memory menu base.
CPadMemoryMenu::~CPadMemoryMenu() {
    ((CFileIoMenu*)this)->Destroy(0);
}
#pragma dont_inline off

// 0x80272F28: tail-call with the caller's owner pointer; the caller at 0x8026C5D0 tests the result at 0x8026C600.
// Destroy every resource owned by the sprite preloader.
int SpritePreload_DestroyOwnedObjects(void* arg) {
    return OwnedObjectList_DestroyAll(arg);
}
// Draw the save menu through its file-menu base.
void CPadMemoryMenuSave::Draw(void* forwarded0) {
    ((CFileIoMenu*)forwarded0)->Draw();
}
// Report that this synchronous preloader is available.
int CPreloadWPuppetSync::vfunc_1() {
    return 1;
}
// Report that this synchronous preloader is ready.
int CPreloadWPuppetSync::IsResourcesReady() {
    return 1;
}
// Create a synchronous file resource for the supplied preload request.
void* CPreloadWPuppetSync::CreateFileSync(void* arg) {
    return CPreloadFileSync::Create(arg);
}
// Create a synchronous sprite resource for the supplied preload request.
void* CPreloadWPuppetSync::CreateSprSync(void* arg) {
    return CPreloadSprSync::Create((const char*)arg);
}
// Create a synchronous sequence resource for the supplied preload request.
void* CPreloadWPuppetSync::CreateSeqSync(void* arg) {
    return CPreloadSeqSync::Create(arg);
}
// Create a synchronous texture resource for the supplied preload request.
void* CPreloadWPuppetSync::CreateTextureSync(void* arg) {
    return CPreloadTextureSync::Create(arg);
}
// Initialize the controller-memory menu unit's angle constant.
void InitPadMemoryMenuAngleConstant() {
    s_padMemoryMenuFullTurn = kFloat_6_2831855;
}
// Report whether the save state machine has reached its terminal state.
int CPadMemoryMenuSave::IsSaveComplete(PadSaveStateView* arg) {
    return arg->saveState == kPadSaveComplete;
}
// Initialize the controller-memory save unit's angle constant.
void InitPadMemorySaveAngleConstant() {
    s_padMemorySaveFullTurn = kFloat_6_2831855;
}
// Initialize the synchronous preload unit's angle constant.
void InitPuppetSyncAngleConstant() {
    s_puppetSyncFullTurn = kFloat_6_2831855;
}

// Initialize the puppet input, then synchronously load its file resources.
int CPreloadWPuppetSync::OpenFileAndLoadResources(void* arg, void* name) {
    if (((CPreloadWPuppet*)arg)->OpenSourceFile(name) == 0)
        return 0;
    return ((CPreloadWPuppet*)arg)->LoadFileResources();
}

#pragma dont_inline on
// Initialize the file-menu base and store this menu's context in the shared global.
int CPadMemoryMenu::SetCurrentSharedContext(void* arg) {
    if (!((CFileIoMenu*)arg)->vfunc_8())
        return 0;
    SharedContext_SetCurrent(AsCPadMemoryMenuSave(arg)->sharedContext); // Context supplied to the menu constructor.
    return 1;
}
#pragma dont_inline off

#pragma dont_inline on
// Construct the file-menu base and remember the controller context. A named member, not a constructor: the derived
// menus do not derive from this class and call it from their own constructors.
void* CPadMemoryMenu::Init(void* value) {
    __ct__11CFileIoMenuFv(this, value);
    controllerIndex = value;
    vtbl = &__vt__14CPadMemoryMenu;
    return this;
}
#pragma dont_inline off

// Query the attached preload file's completion status, or succeed when none exists.
int CPreloadWPuppet::QueryFileCompletion() {
    struct V {
        virtual int a() DECOMP_LOCAL_VIRTUAL;
        virtual int b() DECOMP_LOCAL_VIRTUAL;
        virtual int c() DECOMP_LOCAL_VIRTUAL;
        virtual int d() DECOMP_LOCAL_VIRTUAL;
        virtual int e() DECOMP_LOCAL_VIRTUAL;
    };
    V* p = (V*)file;
    if (p != 0)
        return p->c();
    return 1;
}

#pragma dont_inline on
// Return whether the controller-memory title query succeeds (status zero).
int CPadMemoryMenu::HasStoredTitle() {
    int titlePointer;
    int status = PadMemory_QueryStoredTitle(controllerIndex, &titlePointer);
    return status == 0;
}
#pragma dont_inline off

struct Fn80274274Tag {
    unsigned char byte;
    char pad[3];
};

struct TitleString {
    unsigned int flags;
    unsigned int length;
    unsigned short* data;
    // Initialize an empty inline UTF-16 title string.
    TitleString() : flags(0), length(0), data(0) {}
    // Free title characters only when heap-owned.
    ~TitleString() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return the title length in UTF-16 code units from inline or heap storage.
    unsigned int size() const {
        unsigned int n;
        if (flags >> 31)
            n = length;
        else
            n = *(const unsigned char*)this & 127;
        return n;
    }
};

// Select the shorter UTF-16 range by reference.
inline const unsigned int& MinTitleLength(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

// Read the UTF-16 string length from inline or heap storage.
inline unsigned int GetTitleLength(const TitleString* s) {
    unsigned int n;
    unsigned int f = s->flags >> 31;
    if (f == 0)
        n = *(const unsigned char*)s & 127;
    else
        n = s->length;
    return n;
}

// Compare UTF-16 contents first, then break ties by length.
inline int CompareTitleContents(TitleString* key, TitleString* other, unsigned int& m, unsigned int& b,
                                unsigned int& nc, unsigned int& a) {
    unsigned int n;
    const void* p;
    if (!(other->flags >> 31)) {
        n = *(unsigned char*)other & 127;
        p = (char*)other + 2;
    } else {
        p = other->data;
        n = other->length;
    }
    unsigned int flag = key->flags >> 31;
    nc = n;
    if (flag == 0)
        a = *(unsigned char*)key & 127;
    else
        a = key->length;
    const void* q;
    unsigned int raw;
    if (!(key->flags >> 31)) {
        raw = *(unsigned char*)key & 127;
        q = (char*)key + 2;
    } else {
        q = key->data;
        raw = key->length;
    }
    b = raw;
    m = MinTitleLength(a, b);
    int cmp = wmemcmp((const wchar_t*)q, (const wchar_t*)p, MinTitleLength(nc, m));
    if (cmp == 0) {
        if (m < n)
            return -1;
        else if (m == n)
            return 0;
        else
            return 1;
    }
    return cmp;
}

// Compare two unsigned lengths and return their ordering.
inline int CompareTitleLengths(unsigned int a, unsigned int b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}

// Compare the stored UTF-16 title with this game's expected save title.
// A failed device query returns 0. The borrowed device/title pointers are copied
// into owned engine strings for comparison; lengths count UTF-16 code units,
// range endpoints use bytes (two per code unit). Temporaries free their buffers.
int CPadMemoryMenu::StoredTitleMatchesGame() {
    int titlePointer;
    unsigned int m, b, nc, a;
    if (PadMemory_QueryStoredTitle(controllerIndex, &titlePointer))
        return 0;
    TitleString storedTitle;
    unsigned int storedTitleLength;
    unsigned short* storedTitleText = (unsigned short*)titlePointer;
    storedTitleLength = wcslen((const wchar_t*)storedTitleText);
    WString_ReserveCapacity(&storedTitle, storedTitleLength);
    {
        volatile Fn80274274Tag a;
        Fn80274274Tag b;
        b.byte = a.byte;
        WString_ReplaceRange(&storedTitle, 0, 0, storedTitleText, (char*)storedTitleText + storedTitleLength * 2,
                             &b.byte);
    }
    TitleString expectedTitle;
    unsigned short* gameTitleText = (unsigned short*)g_padSaveGameTitle;
    unsigned int gameTitleLength = wcslen((const wchar_t*)gameTitleText);
    WString_ReserveCapacity(&expectedTitle, gameTitleLength);
    {
        volatile Fn80274274Tag a2;
        Fn80274274Tag b2;
        b2.byte = a2.byte;
        WString_ReplaceRange(&expectedTitle, 0, 0, gameTitleText, (char*)gameTitleText + gameTitleLength * 2, &b2.byte);
    }

    unsigned int titlesMatch = GetTitleLength(&storedTitle) == GetTitleLength(&expectedTitle);
    if (titlesMatch)
        titlesMatch = CompareTitleContents(&storedTitle, &expectedTitle, m, b, nc, a) == 0;
    if (!titlesMatch)
        return 0;
    return 1;
}

// Return the save-record handle, falling back to the current-player profile when absent.
int CPadMemoryMenuSave::GetSaveRecord() {
    int saveRecordOrProfile = (*(int*)&saveData);
    switch (saveRecordOrProfile) {
    case 0:
        saveRecordOrProfile = ((CFileIoMenu*)this)->BuildCurrentPlayerProfile();
        break;
    }
    return saveRecordOrProfile;
}

struct PadSaveUpdateView {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void f4();
    virtual void f5();
    virtual void f6();
    virtual void f7();
    virtual void f8();
    virtual void f9();
    virtual int finishSave();
    virtual void f11();
    virtual void f12();
    virtual void f13();
    virtual void f14();
    virtual void f15();
    virtual void f16();
    virtual void f17();
    virtual void f18();
    virtual void f19();
    virtual void f20();
    virtual void f21();
    virtual void f22();
    virtual void f23();
    virtual void f24();
    virtual void f25();
    virtual void f26();
    virtual void f27();
    virtual void f28();
    virtual void f29();
    virtual void f30();
    virtual void f31();
    virtual void f32();
    virtual void f33();
    virtual int f34_special();
    char base[0x40];
    unsigned int state;
    unsigned int retryCount;
};

// Save ownership/transition map (required live receiver; serialized UI/I/O flow):
// P = menu-owned optional saveData snapshot at +4C. Remains live through EVERY
// state, cancel, error and retry; only InitSaveRecordAndFade replaces it and the
// derived destructor frees it. No I/O operation borrows the caller's init record.
// F = menu-owned saveForm +28; A = menu-owned fileIoAccess +18. Read/write creation
// REPLACES F then A (pad_memory_menu_load.cpp); these calls are not rollback transactions.
// W = A-owned write copy (CFileIoAccessWrite::InitializeBuffer, file_io_flash_and_write_access.cpp):
// CreateWriteFileIo frees its temporary replicated image before return; W stays
// live through open/write/close, freed by access destroy (file_io_access_read_write.cpp).
// R = backing CFileIo-owned aligned read buffer, borrowed through A by verification;
// it is not P and must not be cached past A replacement/release.
// D = base-menu owned modal; all Show/Create dialog APIs manage its replacement;
// DestroyCommonDialog releases it, and base destruction releases any remainder.
// States (P is retained throughout; existing F/A remain until explicitly replaced):
// 0: screen present ->1, absent ->12; no new I/O/modal.
// 1: no title ->6; other title ->2; matching title creates F/A-read ->4.
// 2->3: overwrite D; 3 waits, accepts ->6, declines ->12.
// 4: wait for A terminal; C73D ->2; otherwise create save-confirm D ->5.
// 5: wait for D; accept ->6, decline ->12.
// 6: create progress D, replace F/A with write+W ->7.
// 7: wait for A terminal; disconnect creates D ->8; failure creates D ->11;
//    otherwise replace F/A with read (old W freed) ->9.
// 8: wait for D, then failure D ->11. No write retry.
// 9: wait for read terminal; verify borrowing R -> success D/10; mismatch with
//    retryCount<3 increments retryCount, replaces F/A with write+W ->7;
//    otherwise failure D/11. Increment survives a failed retry creation.
// 10/11: wait for result D ->12 (success/failure share the finish path).
// 12: destroy D; virtual finish hook must succeed before ->13. F/A/P remain.
// 13/unknown state: no derived work; base Update still runs before switch.
// Any base-update, dialog creation, I/O creation or finish-hook zero returns 0
// immediately; state is unchanged at that call site, but partial replacements,
// progress D, guard effects and the retry increment are NOT undone. Caller must
// finish/tear down via the normal lifecycle, not assume return 0 freed resources.
// Callback ownership: CFileIoPad_OnOpenComplete/OnTransferComplete (file_io_pad_and_flash_io.cpp)
// update g_fileIoPad status/byte count and clear busy; they do not own this menu
// or change saveState. Access Update polls open/transfer/close before becoming
// terminal. Keep singleton/transfer storage alive under that access lifecycle;
// do not destroy it manually while callbacks can run. Base destructor releases
// A, F, D and I/O system guards; derived destructor disables fade and frees P.
// Advance save confirmation, asynchronous write, verification, retries, and dialogs.
int CPadMemoryMenuSave::AdvanceSaveStateMachine() {
    PadSaveUpdateView* self = (PadSaveUpdateView*)this;
    if (!((CFileIoMenu*)self)->Update())
        return 0;
    switch (self->state) {
    // Phase 1: choose whether an existing title can be read or needs overwrite approval.
    case kPadSaveCheckScreen:
        if (((CPadMemoryMenu*)self)->HasScreenPointer())
            self->state = kPadSaveCheckExistingTitle;
        else
            self->state = kPadSaveFinish;
        break;
    case kPadSaveCheckExistingTitle:
        if (((CPadMemoryMenu*)self)->HasStoredTitle()) {
            if (((CPadMemoryMenu*)self)->StoredTitleMatchesGame()) {
                if (!((CPadMemoryMenu*)self)->CreateReadFileIo())
                    return 0;
                self->state = kPadSaveWaitRead;
            } else
                self->state = kPadSaveOpenOverwritePrompt;
        } else
            self->state = kPadSaveStartWrite;
        break;
    case kPadSaveOpenOverwritePrompt:
        if (!((CFileIoMenu*)self)->ShowOkCancelDialog(memorySaveDialogMessages.confirmOverwrite))
            return 0;
        self->state = kPadSaveWaitOverwritePrompt;
        break;
    case kPadSaveWaitOverwritePrompt:
        if (!((CFileIoMenu*)self)->IsCommonDialogComplete())
            break;
        if (((CFileIoMenu*)self)->IsCommonDialogAccepted())
            self->state = kPadSaveStartWrite;
        else
            self->state = kPadSaveFinish;
        break;
    case kPadSaveWaitRead:
        if (!((CFileIoMenu*)self)->QueryFileAccessStatus())
            break;
        if (((CPadMemoryMenu*)self)->HasFileErrorC73D())
            self->state = kPadSaveOpenOverwritePrompt;
        else {
            if (!((CFileIoMenu*)self)->ShowOkCancelDialog(memorySaveDialogMessages.confirmSave))
                return 0;
            self->state = kPadSaveWaitSavePrompt;
        }
        break;
    case kPadSaveWaitSavePrompt:
        if (!((CFileIoMenu*)self)->IsCommonDialogComplete())
            break;
        if (((CFileIoMenu*)self)->IsCommonDialogAccepted())
            self->state = kPadSaveStartWrite;
        else
            self->state = kPadSaveFinish;
        break;
    case kPadSaveStartWrite:
        // Phase 2: keep a buttonless progress dialog while the asynchronous write runs.
        if (!((CFileIoMenu*)self)->CreateButtonlessCommonDialog(memorySaveDialogMessages.saving))
            return 0;
        if (!((CPadMemoryMenu*)self)->CreateWriteFileIo())
            return 0;
        self->state = kPadSaveWaitWrite;
        break;
    case kPadSaveWaitWrite:
        if (!((CFileIoMenu*)self)->QueryFileAccessStatus())
            break;
        if (((CPadMemoryMenu*)self)->IsDeviceDisconnected()) {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog(memorySaveDialogMessages.disconnected))
                return 0;
            self->state = kPadSaveWaitDisconnectMessage;
        } else if (((CFileIoMenu*)self)->QueryAccessFailure()) {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog(memorySaveDialogMessages.saveFailed))
                return 0;
            self->state = kPadSaveWaitFailureMessage;
        } else {
            if (!((CPadMemoryMenu*)self)->CreateReadFileIo())
                return 0;
            self->state = kPadSaveWaitVerificationRead;
        }
        break;
    case kPadSaveWaitDisconnectMessage:
        if (!((CFileIoMenu*)self)->IsCommonDialogComplete())
            break;
        if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog(memorySaveDialogMessages.saveFailed))
            return 0;
        self->state = kPadSaveWaitFailureMessage;
        break;
    case kPadSaveWaitVerificationRead:
        // Phase 3: verify readback; only verification mismatch retries the write.
        if (!((CFileIoMenu*)self)->QueryFileAccessStatus())
            break;
        if (((CPadMemoryMenu*)self)->VerifyWrittenRecord()) {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog(memorySaveDialogMessages.saveSucceeded))
                return 0;
            self->state = kPadSaveWaitSuccessMessage;
        } else if (self->retryCount < kPadSaveMaxRetries) {
            ++self->retryCount;
            if (!((CPadMemoryMenu*)self)->CreateWriteFileIo())
                return 0;
            self->state = kPadSaveWaitWrite;
        } else {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog(memorySaveDialogMessages.saveFailed))
                return 0;
            self->state = kPadSaveWaitFailureMessage;
        }
        break;
    case kPadSaveWaitSuccessMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = kPadSaveFinish;
        break;
    case kPadSaveWaitFailureMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = kPadSaveFinish;
        break;
    case kPadSaveFinish:
        // Phase 4: dispose the dialog and wait for the base finish hook before terminal state.
        ((CFileIoMenu*)self)->DestroyCommonDialog();
        if (!self->finishSave())
            return 0;
        self->state = kPadSaveComplete;
        break;
    }
    return 1;
}

struct PadSaveInitDispatch {
    virtual void destroy(int);
    virtual void a();
    virtual void b();
    virtual void c();
    virtual void d();
    virtual void e();
    virtual void f();
    virtual void g();
    virtual void h();
    virtual void i();
    virtual void j();
    virtual void k();
    virtual void l();
    virtual void m();
    virtual void n();
    virtual void o();
    virtual void p();
    virtual void q();
    virtual void r();
    virtual void s();
    virtual void t();
    virtual int init(void*);
};

// Construct the base menu and initialize save state, retries, and record ownership.
inline CPadMemoryMenuSave::CPadMemoryMenuSave(void* s) {
    ((CPadMemoryMenu*)this)->Init(s);
    vtbl = __vt__18CPadMemoryMenuSave;
    saveState = kPadSaveCheckScreen;
    retryCount = 0;
    saveData = 0;
}

// Allocate the save menu in its first state and initialize it with the supplied record.
void* CPadMemoryMenuSave::Create(void* arg) {
    CPadMemoryMenuSave* obj = new CPadMemoryMenuSave(this);
    PadSaveInitDispatch* o = (PadSaveInitDispatch*)obj;
    if (o == 0)
        return 0;
    if (!o->init(arg)) {
        if (o != 0)
            o->destroy(1);
        return 0;
    }
    return o;
}

// Disable the alternate fade overlay and release the owned save-record copy.
CPadMemoryMenuSave::~CPadMemoryMenuSave() {
    int _g = ((int&)Sing_805DC608());
    vtbl = __vt__18CPadMemoryMenuSave;
    if (_g == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_padSaveFadeOverlayDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
    }
    FadeState_SetAlternateDrawEnabled((void*)((int&)Sing_805DC608()), 0);
    if (saveData != 0) {
        __dl__FPv(saveData);
        saveData = 0;
    }
    __dt__14CPadMemoryMenuFv(this, 0);
}

// Copy the optional save-record prefix and enable the save-menu fade overlay.
// arg is borrowed; a non-null record is copied into self-owned 0x47C-byte storage.
// A null arg preserves the existing saveData. Returns 0 on base init/allocation
// failure; the shared fade overlay is enabled only after that work succeeds.
int CPadMemoryMenuSave::InitSaveRecordAndFade(void* arg) {
    if (!CPadMemoryMenu::SetCurrentSharedContext(this))
        return 0;
    if (arg != 0) {
        if (saveData != 0) {
            __dl__FPv(saveData);
            saveData = 0;
        }
        struct Fn80273B50Alloc {
            char data[kPadSaveRecordBytes];
            // Initialize a newly allocated save-record prefix before its contents are copied.
            Fn80273B50Alloc() {
                SaveRecordPrefix_ClearCtor(this);
            }
        };
        Fn80273B50Alloc* obj = new Fn80273B50Alloc;
        saveData = obj;
        if (obj == 0)
            return 0;
        Memory_Clear(obj, kPadSaveRecordBytes);
        SaveData_CopyRecordPrefix47C(saveData, arg);
    }
    if (((int&)Sing_805DC608()) == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_padSaveFadeOverlayDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
    }
    FadeState_SetAlternateDrawEnabled((void*)((int&)Sing_805DC608()), 1);
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046237C", _ctors_u80272E70_0, InitPuppetSyncAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046237C", _ctors_u80272E70_1, InitPadMemoryMenuAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046237C", _ctors_u80272E70_2, InitPadMemorySaveAngleConstant);
#pragma pop

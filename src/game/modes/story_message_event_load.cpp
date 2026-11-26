// Story message loading: the message player (CBpPlayer) that loads and releases its window,
// portrait and speed resources, the asynchronous event file loader (CEventLoad), and the
// static message tree. The loader reads aligned file buffers and retries failed reads.

#include <decomp/static_init.h>
// String-pool view; byte lengths include terminators and original padding.
struct StoryMessageStringPool {
    char messageKeyFile[12];          // +0x0
    char windowLayout[11];            // +0xC
    char textureArchive[17];          // +0x17
    char emptyTexturePrefix[1];       // +0x28
    char speedFile[10];               // +0x29
    char defaultScript[8];            // +0x33
    char outScript[4];                // +0x3B
    char windowOnlySuffix[9];         // +0x3F
    char defaultWindowOnlyScript[16]; // +0x48
    char windowOutScript[17];         // +0x58
    char windowOutNoScrollScript[27]; // +0x69
    char windowInScript[16];          // +0x84
    char inScript[3];                 // +0x94
    char noScrollSuffix[11];          // +0x97
    char displayMessageScript[19];    // +0xA2
    char messageGroup[12];            // +0xB5
    char pageChangeSound[15];         // +0xC1
};
#define MessageStrings (*((StoryMessageStringPool*)storyMessageStrings))

#include "classes/CEventLoad.h"
#define AsCEventLoad(p) ((CEventLoad*)(p))
#include "game/string.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CBpPlayer.h"
#include "classes/CPackTextureResource.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CHashPath.h"
#include "classes/CFileAsync.h"
#include "classes/CBpCtrl.h"

enum EventLoadState {
    kLoadComplete = 0,
    kLoadOpenFile = 1,
    kLoadWaitForSize = 2,
    kLoadWaitForRead = 3
};
enum StoryResourceLimits {
    kInvalidControlEntry = -1,
    kTextureArrayCount = 2,
    kFileAlignmentMask = 31,
    kTextureBindingBytes = 0x28,
    kStringInlineLengthMask = 0x7F,
    kStringHeapTag = 0x80
};
// Lifetime map: loadEvent owns the aligned text-file buffer; sharedFile owns the
// message-key file; fixedFontRenderer borrows the global fixed-font renderer. The two
// MsgEcb construction calls receive output slots at +0/+4; whether they copy file bytes is unresolved.
// controlEntryId registers a BP layout, textureArrays own two arrays of bindings,
// textureResource stages their pack load, and speedFile backs speedController.
// The five script strings retain capacity during ReleaseResources; Dtor frees it.
// CEventLoad states: 1 open -> 2 wait/query size -> 3 wait for read -> 0 complete.
// Success falls through to the next phase in the same poll. Return 0 is pending
// (including retries after errors), not terminal failure; return 1 means complete.

// Partial text-factory view: the returned manager owns the fixed-font renderer.
struct MessageFontFactoryView {
    char reserved00[0x10];
    void* fixedFont;
};

extern "C" {
extern char s_scriptByteKeyMap[];
extern char s_storyConteRequiredFilesStorage[];
extern char s_tourVsWindowDestructorRegistration[];

extern const float kFloat_6_2831855;
extern float s_storyMessageContainersFullTurnAngle;
extern float s_storyMessageFullTurnAngle;

extern void* __nw__FUl(unsigned int);
extern void* StoryMessageTree_DestroyNodes(void*, int);
extern void ScriptByteMap_FreeSubtree(void*, void*);
extern unsigned int String_ReplaceRange(void*, int, unsigned int, void*, void*, void*, void*);

extern void String_ReserveCapacity(void*, int);

extern void __dt__11Vec80121A68Fv();
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor();
extern void __ct__7CBpCtrlFv(void*);
extern void* __ct__8CSpdCtrlFv(void*);
extern void SpdManager_BuildItemsFromResource(void*, void*, int);
extern void* CSharedFilePtr_GetLoadedBuffer(void*);
extern void* NString_AssignCString(void*, const char*);
struct Obj9FB38;
extern void* SharedFile_Create(const char*, int);

extern void* IdMap_Lookup(void*, int);
extern void* CSharedFilePtr_GetLoadedBuffer(void*);
extern void WFixedFont_CreateMsgEcbResource(void*, void*, void*, int);
extern int File_GetLastError();
extern void* appMallocUni__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void __destroy_new_array(void*, void*);
extern void* ResourceTable_FindEntryByName(void*, const char*, void*);

extern signed char s_bpCtrlInitialized;
extern signed char s_pointerPairListInitialized;
extern void* g_chatMsg;
extern unsigned char s_bpCtrl[];
extern unsigned char s_bpCtrlDtorRegistration_StoryMessage[];
extern unsigned char s_messageDrawPairList[];
extern unsigned char s_partyItemVectorDtorRegistration[];
extern void MessageDrawPairList_Dtor();
extern void PairVector_InitReserved(void*);
extern void PointerPairList_RemoveByKey(void*, void*);
extern unsigned char __vt__10CEventLoad[];
extern const double bpPlayerTextureArrayCounts;
// reversed in memory (the Wii reads the bytes in file order).  Wii code is unchanged.
#define BP_TEXTURE_ARRAY_COUNT(i) (((unsigned char*)&bpPlayerTextureArrayCounts)[i])
extern const unsigned char storyMessageStrings[];
extern char s_bpPlayerSharedFilePathBuffer[];
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern const char* const* s_storyPortraitTextureNameRows[2]; // Texture-name rows for the two portrait groups.
extern unsigned char
    s_storyResourceTableDestructorRegistration[]; // Static destructor-registration record for the resource table.
extern "C" void __ct__6CBpTexFv();
extern "C" void __dt__6CBpTexFv(void*);
extern void Allocation_Dtor();
extern void* IdMap_Lookup(void*, int);

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Release message, portrait, layout and speed resources and clear the five message scripts.
void CBpPlayer::ReleaseResources() {
    // Phase 1: delete the async loader, then the +0 wrapper, then the key-file
    // handle and +C wrapper when messageTextContext is present. Preserve this order.
    (*(unsigned int*)&fixedFontRenderer) = 0;
    void* child = loadEvent;
    if (child) {
        if (child) {
            ((void (*)(void*, int))((void**)AsCEventLoad(child)->vtable)[2])(child, 1);
        }
        loadEvent = 0;
    }
    child = *(void**)this;
    if (child) {
        if (child) {
            (*(void (**)(void*, int))((char*)*(void**)child + 8))(child, 1);
        }
        *(void**)this = 0;
    }
    if (messageTextContext) {
        child = sharedFile;
        if (child) {
            CSharedFileHandle::Destroy(child, 1);
            sharedFile = 0;
        }
        child = messageTextContext;
        if (child) {
            if (child) {
                (*(void (**)(void*, int))((char*)*(void**)child + 8))(child, 1);
            }
            messageTextContext = 0;
        }
    }
    // Phase 2: unregister the BP window and release portrait/speed resources.
    if (controlEntryId != kInvalidControlEntry) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(controlEntryId);
        controlEntryId = kInvalidControlEntry;
    }
    for (unsigned char i = 0; i < 2; ++i) {
        void** slot = (void**)((char*)this + (unsigned int)i * 4);
        if (slot[5]) {
            __destroy_new_array(slot[5], (void*)__dt__6CBpTexFv);
            slot[5] = 0;
        }
    }
    child = speedController;
    if (child) {
        if (child) {
            (*(void (**)(void*, int))((char*)*(void**)child + 8))(child, 1);
        }
        speedController = 0;
    }
    child = speedFile;
    if (child) {
        CSharedFileHandle::Destroy(child, 1);
        speedFile = 0;
    }
    // Phase 3: unregister/delete speaker and message draws, then clear speaker state.
    child = speakerNameDraw;
    if (child) {
        if (child) {
            if (0 == ((int&)Sing_805DC650())) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor,
                                             s_partyItemVectorDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), child);
            if (child) {
                if (child) {
                    (*(void (**)(void*, int))((char*)*(void**)child + 8))(child, 1);
                }
            }
        }
        speakerNameDraw = 0;
    }
    speakerIndex = -1;
    speakerTableIndex = -1;
    speakerSlotIndex = -1;
    messageAlpha = 0;
    child = messageDraw;
    if (child) {
        if (child) {
            if (0 == ((int&)Sing_805DC650())) {
                if (s_pointerPairListInitialized == 0) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor,
                                             s_partyItemVectorDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), child);
            if (child) {
                if (child) {
                    (*(void (**)(void*, int))((char*)*(void**)child + 8))(child, 1);
                }
            }
        }
        messageDraw = 0;
    }
    // Phase 4: empty scripts without shrinking/freeing their heap capacity.
    // Inline: byte 0 holds length/tag, byte 1 begins text. Heap: zero the
    // first character and length word, retaining allocation and tag. The first
    // string repeats a terminator store in the original; preserve its schedule.
    advanceRequested = 0;
    // Requested message script: keep capacity, clear text and logical length.
    if ((messageRequestScriptFlags >> 31) == 0) {

        stringBytes_40[1] = 0;
        signed char inlineLengthAndTag = stringBytes_40[0];
        stringBytes_40[1] = 0;
        stringBytes_40[0] = (unsigned char)(inlineLengthAndTag & ~kStringInlineLengthMask);
    } else {
        child = messageRequestScriptData;
        *(unsigned char*)child = 0;
        stringLength_44 = 0;
    }
    // Window-out script: keep capacity, clear text and logical length.
    if ((windowOutScriptFlags >> 31) == 0) {

        stringBytes_4C[1] = 0;
        stringBytes_4C[0] = (unsigned char)((signed char)(stringBytes_4C[0]) & (signed char)kStringHeapTag);
    } else {
        child = windowOutScriptData;
        *(unsigned char*)child = 0;
        stringLength_50 = 0;
    }
    // Window-in script: keep capacity, clear text and logical length.
    if ((windowInScriptFlags >> 31) == 0) {

        stringBytes_58[1] = 0;
        stringBytes_58[0] = (unsigned char)((signed char)(stringBytes_58[0]) & (signed char)kStringHeapTag);
    } else {
        child = windowInScriptData;
        *(unsigned char*)child = 0;
        stringLength_5C = 0;
    }
    // Message-ready script: keep capacity, clear text and logical length.
    if ((messageReadyScriptFlags >> 31) == 0) {

        stringBytes_64[1] = 0;
        stringBytes_64[0] = (unsigned char)((signed char)(stringBytes_64[0]) & (signed char)kStringHeapTag);
    } else {
        child = messageReadyScriptData;
        *(unsigned char*)child = 0;
        stringLength_68 = 0;
    }
    // Message-advance script: keep capacity, clear text and logical length.
    if ((messageAdvanceScriptFlags >> 31) == 0) {

        stringBytes_70[1] = 0;
        stringBytes_70[0] = (unsigned char)((signed char)(stringBytes_70[0]) & (signed char)kStringHeapTag);
    } else {
        child = messageAdvanceScriptData;
        *(unsigned char*)child = 0;
        stringLength_74 = 0;
    }
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
// Texture-pack adapter: first virtual slot releases the staging resource.
class TexturePackTeardown {
public:
    virtual void destroy(int);
};
// Binding readiness is virtual slot 4; preceding slot contracts are unresolved.
class TextureBindingReadiness {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual int isReady();
};
// Wait for layout/portrait resources, then build the character speed controller.
int CBpPlayer::PollResourceLoad() {
    // Return 1 only after speedController exists. Texture pack, all BP entries,
    // both portrait arrays and the speed file must be ready before constructing it.
    if (speedController)
        return 1;
    ((CPackTextureResource*)textureResource)->PollLoad();
    if (!(void*)((CPackTextureResource*)textureResource)->IsLoadFinished())
        return 0;
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    if (!((CBpCtrl*)(((int&)Sing_805DC5F0())))->PollAllEntriesLoaded())
        return 0;
    unsigned char count;
    for (unsigned char i = 0; i < 2; ++i) {
        count = BP_TEXTURE_ARRAY_COUNT(i);
        for (unsigned char j = 0; j < count; ++j) {
            void** arr = (void**)textureArrays[(unsigned int)i];
            arr = (void**)((char*)arr + (unsigned int)j * kTextureBindingBytes);
            void* obj = *arr;
            if (!((TextureBindingReadiness*)arr)->isReady())
                return 0;
        }
    }
    void* speedFileBuffer = CSharedFilePtr_GetLoadedBuffer(speedFile);
    if (!speedFileBuffer)
        return 0;
    // Construct the speed controller with the original new-expression unwind cleanup.
    struct CSpdCtrlStorage {
        char storage[0x30];
        CSpdCtrlStorage() {
            __ct__8CSpdCtrlFv(this);
        }
    };
    void* item = new CSpdCtrlStorage();
    speedController = item;
    SpdManager_BuildItemsFromResource(item, speedFileBuffer, 0);
    if (textureResource) {
        if (textureResource) {
            ((TexturePackTeardown*)textureResource)->destroy(1);
        }
        textureResource = 0;
    }
    return 1;
}
#pragma use_lmw_stmw reset

struct TempString {
    unsigned int flags;
    char data[4];
    void* heap;
};

struct Dt24B {
    char pad[36];
};
class Dt24 : public Dt24B {
public:
    virtual void v08(int);
};
class Dt0 {
public:
    virtual void v08(int);
};
// name is copied into loadEvent after path resolution. flag is forwarded as
// its loadFlags ABI word and as a boolean to SharedFile_Create; arg is passed
// to the constructor's currently unused fourth argument. No async work waits here.
void CBpPlayer::StartLoad(const char* name, int flag, void* arg) {
    fixedFontRenderer = 0;
    void* child = loadEvent;
    if (child) {
        if (child) {
            ((Dt24*)child)->v08(1);
        }
        loadEvent = 0;
    }
    child = vtbl;
    if (child) {
        if (child) {
            ((Dt0*)child)->v08(1);
        }
        vtbl = 0;
    }
    if (messageTextContext) {
        child = sharedFile;
        if (child) {
            CSharedFileHandle::Destroy(child, 1);
            sharedFile = 0;
        }
        child = messageTextContext;
        if (child) {
            if (child) {
                ((Dt0*)child)->v08(1);
            }
            messageTextContext = 0;
        }
    }
    struct StringCleanup {
        TempString* string;
        // Free this temporary's heap text on normal return or exception unwind.
        ~StringCleanup() {
            if (string->flags >> 31)
                appFreeUni__FPv(string->heap);
        }
    };
    TempString first = {};
    StringCleanup firstCleanup = {&first};
    NString_AssignCString(&first, name);
    void* object = __nw__FUl(0x30);
    if (object) {
        unsigned int firstFlag = first.flags >> 31;
        const char* text = !firstFlag ? (const char*)&first + 1 : (const char*)first.heap;
        try {
            ((CEventLoad*)((struct Obj9FB38*)object))->__ct(text, (void*)flag, arg);
        } catch (...) {
            __dl__FPv(object);
            throw;
        }
    }
    loadEvent = object;
    {
        TempString second = {};
        StringCleanup secondCleanup = {&second};
        NString_AssignCString(&second, (const char*)storyMessageStrings);
        unsigned int secondFlag = second.flags >> 31;
        const char* secondText = !secondFlag ? (const char*)&second + 1 : (const char*)second.heap;
        strcpy(s_bpPlayerSharedFilePathBuffer, secondText);
    }
    char* path = s_bpPlayerSharedFilePathBuffer;
    sharedFile = SharedFile_Create(path, flag != 0);
}

struct PathAllocatorTag {
    char c;
};

typedef NString EventLoadPath;

struct Obj9FB38 {
    unsigned int state;
    char asyncFile[0x10];
    EventLoadPath path;
    void* loadFlags;
    void* vtable;
    unsigned int bufferSize; // Padded allocation/read length in bytes, not logical file length.
    unsigned int buffer;
};

// Resolve the requested path and initialize an asynchronous loader in its open-file state.
CEventLoad::CEventLoad(const char* text, void* arg, void* unused) {
    Obj9FB38* loader = (Obj9FB38*)this;
    loader->vtable = __vt__10CEventLoad;
    ((CFileAsync*)loader->asyncFile)->__ct();
    loader->path.flags = 0;
    loader->path.length = 0;
    loader->path.data = 0;
    loader->bufferSize = 0;
    loader->buffer = 0;
    text = (const char*)CHashPath::FindPath(text);
    const char* source = text;
    loader->loadFlags = arg;
    unsigned int oldPathLength;
    if ((loader->path.flags >> 31) == 0)
        oldPathLength = *(unsigned char*)&loader->path & 0x7f;
    else
        oldPathLength = loader->path.length;
    PathAllocatorTag alloc;
    ((void (*)(EventLoadPath*, int, unsigned int, const char*, const char*, PathAllocatorTag))String_ReplaceRange)(
        &loader->path, 0, oldPathLength, source, source + strlen(source), alloc);
    loader->state = kLoadOpenFile;
}

#pragma opt_common_subs off
// Initialize script strings and empty resource slots with no selected speaker or layout.
CBpPlayer::CBpPlayer() {
    void* a40 = ((char*)stringBytes_40);
    windowOutScriptFlags = 0;
    messageRequestScriptFlags = 0;
    stringLength_44 = 0;
    (*(unsigned int*)&messageRequestScriptData) = 0;
    stringLength_50 = 0;
    (*(unsigned int*)&windowOutScriptData) = 0;
    windowInScriptFlags = 0;
    stringLength_5C = 0;
    (*(unsigned int*)&windowInScriptData) = 0;
    messageReadyScriptFlags = 0;
    stringLength_68 = 0;
    (*(unsigned int*)&messageReadyScriptData) = 0;
    messageAdvanceScriptFlags = 0;
    stringLength_74 = 0;
    (*(unsigned int*)&messageAdvanceScriptData) = 0;
    (*(unsigned int*)&fixedFontRenderer) = 0;
    (*(unsigned int*)&loadEvent) = 0;
    *(unsigned int*)this = 0;
    (*(unsigned int*)&messageTextContext) = 0;
    (*(unsigned int*)&controlEntryId) = -1;
    (*(unsigned int*)&textureArrays[0]) = 0;
    (*(unsigned int*)&textureArrays[1]) = 0;
    (*(unsigned int*)&speedController) = 0;
    (*(unsigned int*)&textureResource) = 0;
    (*(unsigned int*)&speakerNameDraw) = 0;
    (*(unsigned int*)&speakerIndex) = -1;
    (*(unsigned int*)&speakerTableIndex) = -1;
    (*(unsigned int*)&speakerSlotIndex) = -1;
    (*(unsigned int*)&messageAlpha) = 0;
    (*(unsigned int*)&messageDraw) = 0;
    advanceRequested = 0;
    try {
        String_ReserveCapacity(a40, 0x40);
        String_ReserveCapacity(((char*)stringBytes_4C), 0x40);
        String_ReserveCapacity(((char*)stringBytes_58), 0x40);
        String_ReserveCapacity(((char*)stringBytes_64), 0x40);
        String_ReserveCapacity(((char*)stringBytes_70), 0x40);
    } catch (...) {
        for (int offset = 0x70; offset >= 0x40; offset -= 12) {
            unsigned int* string = (unsigned int*)((char*)this + offset);
            if (string[0] >> 31)
                appFreeUni__FPv((void*)string[2]);
        }
        throw;
    }
    (*(unsigned int*)&speedFile) = 0;
    (*(unsigned int*)&sharedFile) = 0;
}
#pragma opt_common_subs reset

// Release player resources and the heap buffers belonging to its five script strings.
CBpPlayer::~CBpPlayer() {
    ((CBpPlayer*)this)->ReleaseResources();
    if ((unsigned int)this + 0x70 != 0 && (messageAdvanceScriptFlags >> 31) != 0)
        appFreeUni__FPv(messageAdvanceScriptData);
    if ((unsigned int)this + 0x64 != 0 && (messageReadyScriptFlags >> 31) != 0)
        appFreeUni__FPv(messageReadyScriptData);
    if ((unsigned int)this + 0x58 != 0 && (windowInScriptFlags >> 31) != 0)
        appFreeUni__FPv(windowInScriptData);
    if ((unsigned int)this + 0x4c != 0 && (windowOutScriptFlags >> 31) != 0)
        appFreeUni__FPv(windowOutScriptData);
    if ((unsigned int)this + 0x40 != 0 && (messageRequestScriptFlags >> 31) != 0)
        appFreeUni__FPv(messageRequestScriptData);
}

struct EventLoadDestroyView {
    char stateBytes[4];
    void* asyncFile;
    char asyncFileTail[0x0C];
    unsigned int nameFlags;
    char nameLengthBytes[4];
    void* nameHeap;
    void* loadFlags;
    void* vtable;
    void* bufferSize;
    void* buffer;
};

#pragma opt_common_subs off
// Open, allocate and read an aligned file buffer; failures retry and unfinished work returns zero.
int CEventLoad::Poll() {
    if (state == kLoadComplete)
        return 1;
    switch (state) {
    case kLoadOpenFile: {
        unsigned int flag = nameFlags >> 31;
        void* name = !flag ? (char*)&nameFlags + 1 : nameHeap;
        if (!((CFileAsync*)((char*)asyncFile))->Open((const char*)name, (int)loadFlags)) {
            state = kLoadOpenFile;
            break;
        }
        bufferSize = 0;
        void* old = this->buffer;
        if (old) {
            appFreeUni__FPv(old);
            this->buffer = 0;
        }
        state = kLoadWaitForSize;
    }
    case kLoadWaitForSize: {
        unsigned int status = 0;
        if (!((CFileAsync*)((char*)asyncFile))->PollRead((int*)&status, 0, 0)) {
            if (File_GetLastError())
                state = kLoadOpenFile;
            break;
        }
        if (!status)
            break;
        unsigned int size = ((CFileAsync*)((char*)asyncFile))->GetSize();
        bufferSize = size;
        if (size == 0xFFFFFFFF) {
            bufferSize = 0;
            state = kLoadOpenFile;
            break;
        }
        // Allocate and read a multiple of 32 bytes. Completion compares against
        // this padded length, so the async file layer must report compatible totals.
        bufferSize = (size + kFileAlignmentMask) & ~(unsigned int)kFileAlignmentMask;
        void* buffer = appMallocUni__FUl(bufferSize);
        this->buffer = buffer;
        if (!buffer) {
            bufferSize = 0;
            state = kLoadOpenFile;
            break;
        }
        if (!((CFileAsync*)((char*)asyncFile))->StartRead(buffer, bufferSize)) {
            state = kLoadOpenFile;
            break;
        }
        state = kLoadWaitForRead;
    }
    case kLoadWaitForRead: {
        unsigned int read = 0;
        unsigned int total = 0;
        if (!((CFileAsync*)((char*)asyncFile))->PollRead((int*)&read, (int*)&total, 0)) {
            if (!File_GetLastError())
                break;
            bufferSize = 0;
            void* buffer = this->buffer;
            if (buffer) {
                appFreeUni__FPv(buffer);
                this->buffer = 0;
            }
            ((CFileAsync*)((char*)asyncFile))->Close();
            state = kLoadOpenFile;
            break;
        }
        if (!read)
            break;
        if (bufferSize != total) {
            bufferSize = 0;
            void* buffer = this->buffer;
            if (buffer) {
                appFreeUni__FPv(buffer);
                this->buffer = 0;
            }
            ((CFileAsync*)((char*)asyncFile))->Close();
            state = kLoadOpenFile;
            break;
        }
        // The buffer is a fresh copy of the file (event/area*.obj scripts, tour message .ecb tables), in file (big-endian)
        // byte order.
        ((CFileAsync*)((char*)asyncFile))->Close();
        state = kLoadComplete;
        return 1;
    }
    }
    // Original 0x8009F7F0 unit: all unfinished states share this pending result.
    return 0;
}
#pragma opt_common_subs reset

struct VObjA07A0 {
    virtual void m0(int);
    virtual void m1(const char*);
};

struct StrA07A0 {
    union {
        unsigned int flags;
        unsigned char b0;
    };
    unsigned int len;
    void* heap;
    // Release temporary heap text after texture-path requests have copied it.
    ~StrA07A0() {
        if (flags >> 31)
            appFreeUni__FPv(heap);
    }
};
struct AllocA07A0 {
    char c;
};

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Load the story window, portrait textures and character speed file.
void CBpPlayer::LoadMessageResources() {
    char* base;
    unsigned char n;
    unsigned char count;
    void* array;
    unsigned char j;
    unsigned char i;
    const char* name;
    const char* const* textureNames;
    const char* item;
    void* dest;
    const char* text;
    unsigned int index;
    // Phase 1: remove the old window entry, portrait arrays and speed controller.
    if (controlEntryId != kInvalidControlEntry) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(controlEntryId);
        controlEntryId = kInvalidControlEntry;
    }
    for (unsigned char i = 0; i < 2; ++i) {
        void** slot = (void**)((char*)this + (unsigned int)i * 4);
        if (slot[5]) {
            __destroy_new_array(slot[5], (void*)__dt__6CBpTexFv);
            slot[5] = 0;
        }
    }
    if (speedController) {
        if (speedController) {
            ((VObjA07A0*)speedController)->m0(1);
        }
        speedController = 0;
    }
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))->CreateEntry(MessageStrings.windowLayout, &controlEntryId, 0);

    // Phase 2: request a new window and packed portrait archive.
    unsigned int first = 0;
    if (!((unsigned int&)Sing_805DC620())) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     s_storyResourceTableDestructorRegistration);
            s_resourceTableInitialized = 1;
        }
        ((unsigned int&)Sing_805DC620()) = (unsigned int)&s_resourceTableStorage;
    }
    void* object;
    void* r17 =
        ResourceTable_FindEntryByName((void*)((unsigned int&)Sing_805DC620()), MessageStrings.textureArchive, &first);
    if (textureResource) {
        if (textureResource) {
            ((VObjA07A0*)textureResource)->m0(1);
        }
        textureResource = 0;
    }
    object = __nw__FUl(0x20);
    if (object) {
        try {
            ((CPackTextureResource*)object)
                ->Init((const char*)(MessageStrings.textureArchive), (void**)first, (unsigned int)((unsigned int)r17));
        } catch (...) {
            __dl__FPv(object);
            throw;
        }
    }
    textureResource = object;
    // Phase 3: construct two owned binding arrays and request each named texture.
    // Names are copied during binding initialization; the temporary string is local.
    StrA07A0 second = {};
    base = (char*)storyMessageStrings;
    for (i = 0; i < 2; ++i) {
        count = BP_TEXTURE_ARRAY_COUNT(i);
        array = __nwa__FUl((unsigned int)count * 0x28 + 0x10);
        if (array)
            array = __construct_new_array(array, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, 0x28, count);
        void** slot = (void**)((char*)this + (unsigned int)i * 4);
        slot[5] = array;
        if (count) {
            n = BP_TEXTURE_ARRAY_COUNT(i);
            name = ((StoryMessageStringPool*)base)->emptyTexturePrefix;
            AllocA07A0 a2;
            AllocA07A0 a1;
            for (j = 0; j < n; ++j) {
                index = (second.flags >> 31) == 0 ? (second.b0 & 0x7f) : second.len;
                ((void (*)(StrA07A0*, int, unsigned int, const char*, const char*, AllocA07A0))String_ReplaceRange)(
                    &second, 0, index, name, name + strlen(name), a1);
                textureNames = s_storyPortraitTextureNameRows[i];
                item = textureNames[j];
                index = (second.flags >> 31) == 0 ? (second.b0 & 0x7f) : second.len;
                ((void (*)(StrA07A0*, int, unsigned int, const char*, const char*, AllocA07A0))String_ReplaceRange)(
                    &second, index, 0, item, item + strlen(item), a2);
                dest = (char*)slot[5] + j * 0x28;
                text = (second.flags >> 31) == 0 ? (char*)&second + 1 : (const char*)second.heap;
                ((VObjA07A0*)dest)->m1(text);
            }
        }
    }
    // Phase 4: retain the speed-file handle; PollResourceLoad builds its controller later.
    speedFile = SharedFile_Create(MessageStrings.speedFile, 0);
}
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

// Mixed static area: +3C tree registration, +48 tree (count/root/sentinel),
// +54 vector registration, +60 vector (begin/size/capacity). The shared symbol
// name describes only the first registration; it is not an input-manager object.
struct GlobalObjectArea {
    char destructorRegistrations[0x6C];
};
extern GlobalObjectArea s_storyWindowStaticStorage;

// Return the word stored by this scalar accessor.
unsigned int StoryMessageScalar_GetValue(unsigned int* p) {
    return *p;
}

// Initialize this message module angle constant to one full turn.
void StoryMessage_InitFullTurnAngle() {
    s_storyMessageFullTurnAngle = kFloat_6_2831855;
}

// Release the loaded file buffer, path string and asynchronous-file state.
CEventLoad::~CEventLoad() {
    EventLoadDestroyView* loader = (EventLoadDestroyView*)this;
    loader->vtable = __vt__10CEventLoad;
    loader->bufferSize = 0;
    if (loader->buffer) {
        appFreeUni__FPv(loader->buffer);
        loader->buffer = 0;
    }
    if ((unsigned int)loader + 0x14 != 0 && (loader->nameFlags >> 31) != 0)
        appFreeUni__FPv(loader->nameHeap);
    ((CFileAsync*)&loader->asyncFile)->~CFileAsync();
}

// Wait for both message files and attach them to the shared fixed-font renderer.
int EventText_PollMessageResources(CBpPlayer* player) {
    // Both buffers remain backed by retained owners after this call. fixedFontRenderer
    // is a readiness marker as well as a borrowed renderer pointer, not the file buffer.
    if (player->fixedFontRenderer)
        return 1;
    if (!((CEventLoad*)(player->loadEvent))->Poll())
        return 0;
    void* sharedFileBuffer = CSharedFilePtr_GetLoadedBuffer(player->sharedFile);
    if (!sharedFileBuffer)
        return 0;
    void* manager = g_chatMsg;
    player->fixedFontRenderer = ((MessageFontFactoryView*)manager)->fixedFont;
    void* item = AsCEventLoad(player->loadEvent)->buffer;
    WFixedFont_CreateMsgEcbResource(player->fixedFontRenderer, player, item, 0);
    sharedFileBuffer = CSharedFilePtr_GetLoadedBuffer(player->sharedFile);
    WFixedFont_CreateMsgEcbResource(player->fixedFontRenderer, (char*)player + 4, sharedFileBuffer, 1);
    return 1;
}

// Resolve the current message-window layout, or return null when no entry is assigned.
int CBpPlayer::GetControlEntry() {
    if (controlEntryId != kInvalidControlEntry) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        return (int)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), controlEntryId);
    }
    return 0;
}

// Initialize module angle storage and register cleanup for the static tree and vector.
void StoryMessage_InitializeStaticContainers() {
    float fullTurn = kFloat_6_2831855;
    unsigned int zero = 0;
    unsigned char* storage = (unsigned char*)&s_storyWindowStaticStorage;
    void* treeDestructor = (void*)StoryMessageTree_DestroyNodes;
    unsigned int* tree = (unsigned int*)((storage + 0x48));
    s_storyMessageContainersFullTurnAngle = fullTurn;
    unsigned int* rootSentinel = tree + 1;
    unsigned int* treeRegistration = (unsigned int*)((storage + 0x3C));
    tree[0] = zero;
    tree[1] = zero;
    tree[2] = (unsigned int)rootSentinel;
    __register_global_object(tree, treeDestructor, treeRegistration);
    unsigned int* vector = (unsigned int*)((storage + 0x60));
    vector[0] = zero;
    void* vectorDestructor = (void*)__dt__11Vec80121A68Fv;
    unsigned int* vectorRegistration = (unsigned int*)((storage + 0x54));
    vector[1] = zero;
    vector[2] = zero;
    __register_global_object(vector, vectorDestructor, vectorRegistration);
}

struct Node874 {
    Node874* left;
    Node874* right;
};

#pragma use_lmw_stmw on
#pragma push
#pragma always_inline on
// Free a node after its children; expand 1 local level(s), then recurse.
inline void DestroyMessageTreeLevel1(void* owner, Node874* const& node) {
    if (node->left)
        ScriptByteMap_FreeSubtree(owner, node->left);
    if (node->right)
        ScriptByteMap_FreeSubtree(owner, node->right);
    appFreeUni__FPv(node);
}
#pragma pop
// Free the static tree nodes in postorder and optionally delete the tree container.
void* StoryMessageTree_DestroyNodes(void* self, int freeSelf) {
    Node874* root;
    if (self != 0) {
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            root = *(Node874**)((char*)self + 4);
                            if (root != 0) {
                                {
                                    Node874* left4 = root->left;
                                    if (left4) {
                                        {
                                            Node874* left3 = left4->left;
                                            if (left3) {
                                                {
                                                    Node874* left2 = left3->left;
                                                    if (left2) {
                                                        DestroyMessageTreeLevel1(self, left2);
                                                    }
                                                    Node874* right2 = left3->right;
                                                    if (right2) {
                                                        DestroyMessageTreeLevel1(self, right2);
                                                    }
                                                    appFreeUni__FPv(left3);
                                                }
                                            }
                                            Node874* right3 = left4->right;
                                            if (right3) {
                                                {
                                                    Node874* left2 = right3->left;
                                                    if (left2) {
                                                        DestroyMessageTreeLevel1(self, left2);
                                                    }
                                                    Node874* right2 = right3->right;
                                                    if (right2) {
                                                        DestroyMessageTreeLevel1(self, right2);
                                                    }
                                                    appFreeUni__FPv(right3);
                                                }
                                            }
                                            appFreeUni__FPv(left4);
                                        }
                                    }
                                    Node874* right4 = root->right;
                                    if (right4) {
                                        {
                                            Node874* left3 = right4->left;
                                            if (left3) {
                                                {
                                                    Node874* left2 = left3->left;
                                                    if (left2) {
                                                        DestroyMessageTreeLevel1(self, left2);
                                                    }
                                                    Node874* right2 = left3->right;
                                                    if (right2) {
                                                        DestroyMessageTreeLevel1(self, right2);
                                                    }
                                                    appFreeUni__FPv(left3);
                                                }
                                            }
                                            Node874* right3 = right4->right;
                                            if (right3) {
                                                {
                                                    Node874* left2 = right3->left;
                                                    if (left2) {
                                                        DestroyMessageTreeLevel1(self, left2);
                                                    }
                                                    Node874* right2 = right3->right;
                                                    if (right2) {
                                                        DestroyMessageTreeLevel1(self, right2);
                                                    }
                                                    appFreeUni__FPv(right3);
                                                }
                                            }
                                            appFreeUni__FPv(right4);
                                        }
                                    }
                                    appFreeUni__FPv(root);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (freeSelf > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046205C", _ctors_u8009F7F0_0, StoryMessage_InitializeStaticContainers);
STATIC_INIT_ENTRY(".CRT$XCU8046205C", _ctors_u8009F7F0_1, StoryMessage_InitFullTurnAngle);
#pragma pop

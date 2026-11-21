#include <decomp/static_init.h>
extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Borrow the string representation and arm exception cleanup until ownership is committed.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Leave the completed string owned by its enclosing resource.
    void release() { active = false; }
    // Free heap storage only while this guard still owns unwind responsibility.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u)) appFreeUni__FPv((void*)words[2]);
    }
};

#include "classes/CPackTextureResource.h"
#include "classes/CNowReloading.h"

#define AsCNowReloading(p) ((CNowReloading*)(p))

#define AsCPackTextureResource(p) ((CPackTextureResource*)(p))
// Translation unit 0x8028F4D0-0x8028FE2C (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u8028F4D0 {
    const unsigned int* words;
    bool active;
    // Arm cleanup for a completed temporary string; the representation is borrowed.
    explicit StringUnwindCleanup_u8028F4D0(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disarm after the caller explicitly frees or transfers the string storage.
    void release() { active = false; }
    // Release an armed heap string on normal or exceptional scope exit.
    ~StringUnwindCleanup_u8028F4D0() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};


void* CPreloadFileAsync_Create(void*);
extern float kFloat_6_2831855;
extern float s_packTextureFullTurnAngle;
extern float s_texturePreloadFullTurnAngle;
extern char __vt__13CNowReloading[];
extern char s_cLocaleData[];
void CPreloadTexture_dtor(void*, int);
void __dl__FPv(void*);
unsigned int strlen(const char*);
void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, unsigned char*);
void* __nwa__FUl(unsigned int);
void String_ReserveCapacity(void*, unsigned int);
void* CreateSharedTextureResource__FPCci(void*, int);
void* SharedFile_Create(void*, int);
int CSharedFilePtr_GetLoadedBuffer(void*);
void* CSharedFilePtr_GetSize(void*);
void CSharedFileHandle_Destroy(void*, int);
int CPackTextureResource_LoadArchiveEntries(void*, void*, void*);
// Compare a texture entry's basename with the requested name, ignoring locale-aware case.
int CPackTextureResource_EntryBasenameMatches(void*, unsigned int, void*);
extern char __vt__20CPackTextureResource[];
extern char packTextureArchiveMemberNameFormat[];
void* Archive_Open(void*, void*, int);
void Archive_GetEntryInfo(void*, int, void*);
void* MSLString_LowercaseInPlace(void*);
char* strrchr(const char*, int);
int strcmp(void*, void*);
void sprintf(void*, const char*, ...);
void* appMallocUni__FUl(const char*);
void Archive_ReadEntry(void*, int, void*, void*, int);
void Archive_Close(void*);
int Get__15ISharedResourceFv(void*);
void CreateOnMemoryResource__22CSharedTextureResourceFPvUl(void*, void*, void*);
void appFreeUni__FPv(void*);
void __dla__FPv(void*);
extern char __vt__20CPackTextureResource[];

struct VirtSlots {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
};

// ABI view of the packed texture resource: path string, texture handles, names, and shared file.
struct PackTextureResourceState {
    void* vtable;
    void** sharedTextures;
    unsigned int stringFlags;
    unsigned int stringLength;
    void* stringData;
    unsigned int textureNamesAddress;
    unsigned int textureCount;
    void* sharedFile;
};

struct StringValue {
    int prefix0;
    int prefix4;
    unsigned int stringFlags;
    unsigned int stringLength;
};

struct PackTextureInitState {
    void* vtable;
    void* textureArray;
    unsigned int stringFlags;
    unsigned int stringLength;
    char* stringData;
    void** textureNames;
    unsigned int textureCount;
    void* sharedFile;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
// Release the archive handle, path storage, shared texture children, and optional object allocation.
void* CPackTextureResource_dtor(PackTextureResourceState* self, int del) {
    if (self != 0) {
        self->vtable = __vt__20CPackTextureResource;
        if (self->sharedFile != 0) {
            CSharedFileHandle_Destroy(self->sharedFile, 1);
            self->sharedFile = 0;
        }
        if ((self->stringFlags >> 31) == 0) {
            int c;
            int zero;
            c = *(unsigned char*)&self->stringFlags;
            zero = 0;
            AsCPackTextureResource(self)->inlinePath = zero;
            c = c & ~0x7F;
            *(unsigned char*)&self->stringFlags = (unsigned char)c;
        } else {
            *(unsigned char*)self->stringData = 0;
            self->stringLength = 0;
        }
        for (unsigned int i = 0; i < self->textureCount; ++i) {
            void* child = self->sharedTextures[i];
            if (child != 0) {
                ((VirtSlots*)child)->slot1();
                self->sharedTextures[i] = 0;
            }
        }
        if (self->sharedTextures != 0) {
            __dla__FPv(self->sharedTextures);
            self->sharedTextures = 0;
        }
        if (((char*)self + 8) != 0 && (self->stringFlags >> 31) != 0)
            appFreeUni__FPv(self->stringData);
        if (del > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma pop

// Archive metadata view over tempName below: name +0, size word +0x124, tail +0x128 (0x134 bytes).
// byteCount is an integer byte size carried in a pointer-typed ABI word, never a buffer address.
// Archive_GetEntryInfo receives &total, the preceding stack word: entry names begin at +4.
// This split stack record is compiler-sensitive; tempName alone is not the complete output record.
struct ArchiveEntryInfo { char name[0x124]; void* byteCount; char trailing[0xC]; };
#define LOCALE_FIELD_OFFSET(field) ((int)&((LocaleDataView*)0)->field)
#define FACET_FIELD_OFFSET(field) ((int)&((LowercaseFacet*)0)->field)
struct LowercaseFacet { char prefix[0x10]; unsigned char* lowercase; };
struct LocaleDataView { char prefix[0x38]; LowercaseFacet* characterFacet; };
enum TextureArchiveConstants { kArchiveReadMode = 3, kArchiveEntryCount = -1, kInlineStringLengthMask = 0x7f };

struct StrObj {
    unsigned int raw;
    unsigned int len;
    char* buf;
    // Test the string ownership flag without inspecting inline character bytes.
    bool isHeap() const { return (raw >> 31) != 0; }
};

// Compare the indexed entry's slash-stripped basename to borrowed namePtr, returning 1 on equality.
// The requested name is used as supplied (no slash stripping); only owned temporary copies are folded.
int CPackTextureResource_EntryBasenameMatches(void* self, unsigned int index, void* namePtr) {
    unsigned int entryNameLength;
    unsigned int requestedNameLength;
    char* str = ((char**)((PackTextureInitState*)self)->textureNames)[index];
    char* hit = strrchr(str, '/');
    if (hit != 0)
        str = hit + 1;

    StrObj entryName = {0, 0, 0};
    char flag1[8];
    entryNameLength = strlen(str);
    String_ReserveCapacity(&entryName, entryNameLength);
    // Empty allocator-tag copy, not string flags or comparison state; the original source byte
    // is uninitialized. Keep this separate stack slot/lifetime to preserve the library ABI.
    flag1[0] = flag1[4];
    String_ReplaceRange(&entryName, 0, 0, str, str + entryNameLength, (unsigned char*)flag1);
    StringUnwindCleanup_u8028F4D0 stringCleanup0(&entryName);

    StrObj requestedName = {0, 0, 0};
    char flag2[8];
    requestedNameLength = strlen((const char*)namePtr);
    String_ReserveCapacity(&requestedName, requestedNameLength);
    // Same empty allocator-tag copy as flag1; neither byte carries initialized application state.
    flag2[0] = flag2[4];
    String_ReplaceRange(&requestedName, 0, 0, (const char*)namePtr, (const char*)namePtr + requestedNameLength, (unsigned char*)flag2);

    // Phase 1: lowercase the private entry basename through the current locale's byte facet.
    unsigned int entryNameIsHeap = entryName.raw >> 31;
    unsigned int entryLength;
    unsigned char* entryBytes;
    if (entryNameIsHeap == 0) {
        entryBytes = (unsigned char*)&entryName + 1;
        entryLength = *(unsigned char*)&entryName & kInlineStringLengthMask;
    } else {
        entryBytes = (unsigned char*)entryName.buf;
        entryLength = entryName.len;
    }
    bool entryNameIsInline = entryNameIsHeap == 0;
    unsigned char* entryEnd = entryBytes + entryLength;
    unsigned char* s;
    signed char* entryDestination = entryNameIsInline ? (signed char*)((char*)&entryName + 1) : (signed char*)entryName.buf;
    for (s = entryNameIsInline ? (unsigned char*)((char*)&entryName + 1) : (unsigned char*)entryName.buf; s != entryEnd; ++s, ++entryDestination) {
        bool oob;
        int c;
        c = (signed char)*s;
        oob = false;
        oob = (c < 0 || c >= 0x100) ? true : oob;
        c = oob ? c : (*(unsigned char**)((char*)*(unsigned int**)(s_cLocaleData + LOCALE_FIELD_OFFSET(characterFacet)) + FACET_FIELD_OFFSET(lowercase)))[c];
        *entryDestination = c;
    }

    // Phase 2: lowercase the private requested-name copy using the same facet.
    unsigned int requestedNameIsHeap = requestedName.raw >> 31;
    unsigned int requestedLength;
    unsigned char* requestedBytes;
    if (requestedNameIsHeap == 0) {
        requestedBytes = (unsigned char*)&requestedName + 1;
        requestedLength = *(unsigned char*)&requestedName & kInlineStringLengthMask;
    } else {
        requestedBytes = (unsigned char*)requestedName.buf;
        requestedLength = requestedName.len;
    }
    bool requestedNameIsInline = requestedNameIsHeap == 0;
    unsigned char* requestedEnd = requestedBytes + requestedLength;
    signed char* requestedDestination = requestedNameIsInline ? (signed char*)((char*)&requestedName + 1) : (signed char*)requestedName.buf;
    for (s = requestedNameIsInline ? (unsigned char*)((char*)&requestedName + 1) : (unsigned char*)requestedName.buf; s != requestedEnd; ++s, ++requestedDestination) {
        bool oob;
        int c;
        c = (signed char)*s;
        oob = false;
        oob = (c < 0 || c >= 0x100) ? true : oob;
        c = oob ? c : (*(unsigned char**)((char*)*(unsigned int**)(s_cLocaleData + LOCALE_FIELD_OFFSET(characterFacet)) + FACET_FIELD_OFFSET(lowercase)))[c];
        *requestedDestination = c;
    }

    // Phase 3: compare the folded strings, then free only heap-backed temporary storage.
    bool requestedNameUsesHeap = requestedName.isHeap();
    int nameComparison = strcmp((entryName.raw >> 31) == 0 ? (char*)((char*)&entryName + 1) : entryName.buf,
                         !requestedNameUsesHeap ? (char*)((char*)&requestedName + 1) : requestedName.buf);
    if (nameComparison == 0) {
        if (requestedNameUsesHeap) appFreeUni__FPv(requestedName.buf);
        if (entryName.isHeap()) { stringCleanup0.release(); appFreeUni__FPv(entryName.buf); }
        return 1;
    }
    if (requestedNameUsesHeap) appFreeUni__FPv(requestedName.buf);
    if (entryName.isHeap()) { stringCleanup0.release(); appFreeUni__FPv(entryName.buf); }
    return 0;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Read archive members into their matching shared textures and discard textures left unloaded.
// file/section are the borrowed archive memory and size word. Return 1 iff every non-null
// requested texture is loaded after the sweep; an archive-open failure returns 0 immediately.
int CPackTextureResource_LoadArchiveEntries(void* self, void* file, void* section) {
    PackTextureResourceState* u = (PackTextureResourceState*)self;
// Archive_GetEntryInfo fills one record starting at `total`: count word, then the name area and the size words.
    char tempName[0x134];
    void* entryByteCount;
    int resourceOrStatus;
    void* handle;
    unsigned int off;
    unsigned int idx;
    int ok;
    int total;
    int count;
    const char* value;
    int i;
    handle = Archive_Open(file, section, kArchiveReadMode);
    if (handle == 0)
        return 0;
    // Phase 1: query member count, then reuse the split {total, tempName} metadata record.
    Archive_GetEntryInfo(handle, kArchiveEntryCount, &total);
    count = total;
    resourceOrStatus = 1;
    i = 0;
    for (; i < count; ++i) {
        Archive_GetEntryInfo(handle, i, &total);
        void* name = MSLString_LowercaseInPlace(tempName);
        sprintf(tempName, packTextureArchiveMemberNameFormat, name);
        // Allocate exactly the metadata byte count. Pointer casts preserve the existing ABI declaration.
        value = (const char*)appMallocUni__FUl((const char*)((ArchiveEntryInfo*)tempName)->byteCount);
        Archive_ReadEntry(handle, i, (void*)value, ((ArchiveEntryInfo*)tempName)->byteCount, kArchiveReadMode);
        entryByteCount = ((ArchiveEntryInfo*)tempName)->byteCount;
        unsigned int index = 0;
        // Phase 2: find a matching requested texture. resourceOrStatus is a reused register:
        // 1 before search, a texture pointer on match, 0 only on search exhaustion, then 1 again.
        // It is not a texture pointer outside the match branch. Preserve the exhaustion assignment.
        for (; index < u->textureCount || (resourceOrStatus = 0, false); ++index) {
            if (CPackTextureResource_EntryBasenameMatches(self, index, tempName)) {
                resourceOrStatus = (int)u->sharedTextures[index];
                break;
            }
        }
        if (resourceOrStatus == 0)
            resourceOrStatus = 1;
        else {
            if (!Get__15ISharedResourceFv((void*)resourceOrStatus))
                CreateOnMemoryResource__22CSharedTextureResourceFPvUl((void*)resourceOrStatus, (void*)value, entryByteCount);
            resourceOrStatus = 1;
        }
        // The memory-resource call finishes before this temporary entry buffer is freed.
        if (value != 0)
            appFreeUni__FPv((void*)value);
        if (resourceOrStatus == 0)
            break;
    }
    // Phase 3: close the archive and release/null any requested child still missing its resource.
    Archive_Close(handle);
    int keep = resourceOrStatus != 0;
    if (keep != 0) {
        ok = 1;
        idx = 0;
        off = 0;
        for (; idx < u->textureCount; ++idx) {
            void* child = *(void**)((char*)u->sharedTextures + off);
            if (child != 0) {
                if (!Get__15ISharedResourceFv(child)) {
                    ok = 0;
                    child = *(void**)((char*)u->sharedTextures + off);
                    if (child != 0) {
                        ((VirtSlots*)child)->slot1();
                        *(void**)((char*)u->sharedTextures + off) = 0;
                    }
                }
            }
            off += 4;
        }
        return ok != 0 ? 1 : 0;
    }
    return keep;
}
#pragma opt_lifetimes reset
#pragma pop

// Poll the shared file, load available textures, then release the file and clear the pending path.
// Return 0 while the file buffer is unavailable, otherwise 1 (load completion, not load success).
int CPackTextureResource_PollLoad(PackTextureInitState* self) {
    unsigned int size;
    if ((self->stringFlags >> 31) == 0)
        size = (AsCPackTextureResource(self)->pathFlags) & kInlineStringLengthMask;
    else
        size = self->stringLength;
    if (size == 0)
        return 1;
    int result = CSharedFilePtr_GetLoadedBuffer(AsCPackTextureResource(self)->sharedFile);
    if (result == 0)
        return 0;
    void* value = CSharedFilePtr_GetSize(AsCPackTextureResource(self)->sharedFile);
    CPackTextureResource_LoadArchiveEntries(self, (void*)result, value);
    void* old = AsCPackTextureResource(self)->sharedFile;
    if (old != 0) {
        CSharedFileHandle_Destroy(old, 1);
        AsCPackTextureResource(self)->sharedFile = 0;
    }
    if ((self->stringFlags >> 31) == 0) {
        AsCPackTextureResource(self)->inlinePath = 0;
        AsCPackTextureResource(self)->pathFlags = AsCPackTextureResource(self)->pathFlags & ~0x7F;
    } else {
        *(unsigned char*)self->stringData = 0;
        self->stringLength = 0;
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
// Create shared textures for all requested names and start loading the containing archive.
// Copies text, borrows the names array and its strings, owns the texture-handle array and file handle.
// Caller must keep values/count stable until destruction; return self without allocating it here.
void* CPackTextureResource_Init(PackTextureInitState* self, const char* text, void** values, unsigned int count) {
    int zero = 0;
    *(void**)self = __vt__20CPackTextureResource;
    self->stringFlags = 0;
    self->stringLength = 0;
    self->stringData = 0;
    EH5StringCleanup eh5Cleanup3(&self->stringFlags);
    AsCPackTextureResource(self)->sharedFile = 0;
    if (text == 0 || values == 0)
        { eh5Cleanup3.release(); return self; }
    char flag[8];
    unsigned int n;
    if (zero == 0)
        n = *(unsigned char*)&self->stringFlags & kInlineStringLengthMask;
    else
        n = 0;
    // Empty allocator-tag byte copy; this is not an initialized path option or string flag.
    flag[0] = flag[4];
    unsigned int length = strlen(text);
    String_ReplaceRange(&self->stringFlags, 0, n, text, text + length, (unsigned char*)flag);
    self->textureNames = values;
    self->textureCount = count;
    self->textureArray = __nwa__FUl(count * 4);
    for (unsigned int i = 0; i < self->textureCount; ++i)
        ((void**)self->textureArray)[i] = CreateSharedTextureResource__FPCci(self->textureNames[i], 1);
    void* s = (self->stringFlags >> 31) == 0 ? (void*)((char*)self + 9) : self->stringData;
    self->sharedFile = SharedFile_Create(s, 0);
    eh5Cleanup3.release(); return self;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma pop

struct ChildBase { char padding[0x78]; };
struct ChildObject : ChildBase { 
virtual ~ChildObject();
};

// Delete the owned child object, restore the reloading vtable, and optionally delete this object.
void* CNowReloading_dtor(void* self, int del) {
    if (self != 0) {
        ChildObject* child = *(ChildObject**)self;
        AsCNowReloading(self)->vtable = __vt__13CNowReloading;
        if (child != 0) {
            delete child;
            *(void**)self = 0;
        }
        if (del > 0)
            __dl__FPv(self);
    }
    return self;
}

// Clear the owned child and install the reloading vtable.
void CNowReloading_Init(void* self) {
    int zero = 0;
    *(int*)self = zero;
    (*(char**)&AsCNowReloading(self)->vtable) = __vt__13CNowReloading;
}

// Test the inline or heap-backed string member for zero length.
int CPackTextureResource_IsLoadFinished(StringValue* self) {
    unsigned int size;
    if ((self->stringFlags >> 31) == 0)
        size = (*(volatile unsigned char*)&self->stringFlags) & kInlineStringLengthMask;
    else
        size = self->stringLength;
    if (size != 0)
        return 0;
    return 1;
}

// Destroy the texture preloader base and optionally delete its allocation.
void* CPreloadTextureAsync_dtor(void* self, int del) {
    if (self != 0) {
        CPreloadTexture_dtor(self, 0);
        if (del > 0)
            __dl__FPv(self);
    }
    return self;
}

// Create the asynchronous shared-file preload request.
void* CPreloadTextureAsync_CreateFilePreload(void*, void* arg) {
    return CPreloadFileAsync_Create(arg);
}

// Initialize the texture-resource unit's full-turn angle constant.
void PackTextureResource_InitFullTurnConstant() {
    s_packTextureFullTurnAngle = kFloat_6_2831855;
}

// Initialize the texture-preload unit's full-turn angle constant.
void TexturePreload_InitFullTurnConstant() {
    s_texturePreloadFullTurnAngle = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462444", _ctors_u8028F4D0_0, PackTextureResource_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462444", _ctors_u8028F4D0_1, TexturePreload_InitFullTurnConstant);
#pragma pop

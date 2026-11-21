typedef unsigned long new_size_t;
// Translation unit 0x8029523C-0x80297160 (13 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#define GAME_WSTRING_DECLARATIONS_ONLY
#include "game/wstring.h"

// Construct a string or tree payload in storage already owned by its container.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {
#pragma use_lmw_stmw on
#pragma scheduling on
#pragma opt_loop_invariants off
#pragma auto_inline off

// Plain line vector, distinct from the file list whose first word is a vtable.
struct Vec12 { void* entries; unsigned int count; unsigned int capacity; };

struct Entries {
    void* vtable;
    void** entries;
    unsigned int count;
    unsigned int capacity;
    // Return the number of file-list slots, including any null slots, not allocation capacity.
    unsigned int size() const { return count; }
};

void* CSharedFilePtr_GetLoadedBuffer(void*);
void* CSharedFilePtr_GetSize(void*);
void TPLBind(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
int wmemcmp(const void*, const void*, unsigned int);
void* SharedFile_Create(void*, int);
void CSharedFileHandle_Destroy(void*, int);
extern void* s_bannerCommentFilename;
extern void* s_bannerIconFilename;
extern void* s_bannerImageFilename;

struct String {
    unsigned int flags;
    unsigned int length;
    void* data;
    // Return UTF-16 code-unit length from the inline tag or heap representation.
    unsigned int size() const { unsigned int n; if (!(flags >> 31)) n = *(const unsigned char*)this & 127; else n = length; return n; }
    // Borrow UTF-16 storage and report its code-unit count; no ownership transfer.
    const void* view(unsigned int& n) const {
        if (!(flags >> 31)) { n = *(const unsigned char*)this & 127; return (const char*)this + 2; }
        else { n = length; return data; }
    }
};
// Release a wide string\'s heap buffer, then optionally delete its storage.
void* WideString_dtor(String* self, int flag) {
    if (self) {
        if (self) {
            if (self->flags >> 31) appFreeUni__FPv(self->data);
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Banner files are ordered text, icon TPL, banner TPL; the ready byte is at 0x20.
struct EntryState { void* vtable; void** entries; unsigned int count; char pad[20]; bool ready; };
enum BannerFileIndex { kBannerTextFile = 0, kBannerIconFile = 1, kBannerImageFile = 2, kBannerFileCount = 3 };
// Bind both loaded TPL images once; report true only on the transition to ready.
int SaveBanner_BindLoadedImages(EntryState* self) {
    if (self->ready) return 0;
    void* p = self->count <= kBannerImageFile ? 0 : self->entries[kBannerImageFile];
    p = p ? CSharedFilePtr_GetLoadedBuffer(p) : 0;
    if (!p) return 0;
    if (!p) return 0;
    TPLBind(p);
    p = self->count <= kBannerIconFile ? 0 : self->entries[kBannerIconFile];
    p = p ? CSharedFilePtr_GetLoadedBuffer(p) : 0;
    if (!p) return 0;
    if (!p) return 0;
    TPLBind(p);
    self->ready = true;
    return 1;
}

struct CleanupArray;
// Parent pointers reserve bit 0 for the red/black color; the value owns a vector of text lines.
struct Node { Node* left; Node* right; unsigned int parentAndColor; String key; CleanupArray* lines; };
struct Tree { unsigned int count; Node* root; Node* first; };
struct BannerStorage {
    Entries files;
    unsigned int reserved10;
    Tree textSections;
    bool imagesReady;
};
typedef char BannerStorage_sections[((int)&((BannerStorage*)0)->textSections == 20) ? 1 : -1];
typedef char BannerStorage_ready[((int)&((BannerStorage*)0)->imagesReady == 32) ? 1 : -1];
// Select the shorter comparison prefix, retaining the reference return convention.
inline const unsigned int& minsize(const unsigned int& a, const unsigned int& b) { return a < b ? a : b; }
#pragma scheduling on
// Compare UTF-16 contents, then break equal-prefix ties by string length.
inline int comparekey(String* key, String* other) {
    unsigned int prefixLength, keyLengthCopy, otherLengthCopy, keyLength;
    unsigned int otherLength;
    const void* otherData;
    if (!(other->flags >> 31)) { otherLength = *(unsigned char*)other & 127; otherData = (char*)other + 2; }
    else { otherData = other->data; otherLength = other->length; }
    unsigned int flag = key->flags >> 31;
    otherLengthCopy = otherLength;
    if (flag == 0) keyLength = *(unsigned char*)key & 127;
    else keyLength = key->length;
    const void* keyData; unsigned int keyLengthSnapshot;
    if (!(key->flags >> 31)) { keyLengthSnapshot = *(unsigned char*)key & 127; keyData = (char*)key + 2; }
    else { keyData = key->data; keyLengthSnapshot = key->length; }
    keyLengthCopy = keyLengthSnapshot;
    prefixLength = minsize(keyLength,keyLengthCopy);
    int cmp = wmemcmp(keyData,otherData,minsize(otherLengthCopy,prefixLength));
    if (cmp == 0) {
        if (prefixLength < otherLength) return -1;
        else if (prefixLength == otherLength) return 0;
        else return 1;
    }
    return cmp;
}
#pragma opt_propagation on
// Find the insertion parent and predecessor for a localized section key.
Node* SaveBannerSectionTree_FindInsertionPosition(Tree* self, String* key, Node** found, unsigned char* left, unsigned char* absent) {
    Node* parent = (Node*)&self->root;
    *found = 0;
    Node* node = self->root;
    *left = 1;
    *absent = 1;
    while (node) {
        parent = node;
        int cmp = comparekey(key, &node->key);
        if (cmp < 0) { node = node->left; *left = 1; }
        else { *found = node; node = node->right; *left = 0; *absent = 0; }
    }
    return parent;
}
#pragma opt_common_subs off
#pragma scheduling on
#pragma opt_propagation on

// Return the line-vector count at +4 through the existing C symbol (a 32-bit ABI word).
// Every use passes a Vec12; no file-list Entries view or pointer-valued count is needed.
unsigned int Container_ReadWordAtOffset4(const Vec12* self) {
    return self->count;
}

// Return a loaded file buffer, or null for an invalid/unavailable entry.
void* SharedFileList_GetLoadedBufferAt(Entries* self, int index) {
    void* entry;
    if (index < 0 || self->count <= (unsigned int)index)
        entry = 0;
    else
        entry = self->entries[index];
    if (!entry)
        return 0;
    return CSharedFilePtr_GetLoadedBuffer(entry);
}

// Query the file-size word for a valid shared-file entry; invalid entries return null.
void* SharedFileList_GetFileSizeAt(Entries* self, int index) {
    void* entry;
    if (index < 0 || self->count <= (unsigned int)index)
        entry = 0;
    else
        entry = self->entries[index];
    if (!entry)
        return 0;
    return CSharedFilePtr_GetSize(entry);
}

void WString_ReserveCapacity(String*, unsigned int);
void WString_ReplaceRange(String*, int, int, void*, void*, unsigned char*);
extern unsigned int s_bannerDefaultSectionValue;
Node* SaveBannerSectionMap_InsertNode(Tree*, Node*, int, int, void*);
struct Tag { unsigned char byte; char pad[3]; };
void __throw(void*, void*, void (*)()) __attribute__((noreturn));
struct KeyValue {
    String key;
    unsigned int value;
    // Duplicate an owned UTF-16 key; the caller assigns its mapped value separately.
    KeyValue(const String& s) {
        register String* dest = &key;
        unsigned int flags = s.flags;
        if (!(flags >> 31)) {
            unsigned int len = s.length;
            void* c = s.data;
            *(volatile unsigned int*)&key.flags = flags;
            *(volatile unsigned int*)&key.length = len;
            *(void* volatile*)&key.data = c;
        }
        else {
            dest->flags = 0;
            unsigned int reserve = s.length;
            dest->length = 0; dest->data = 0;
            WString_ReserveCapacity(dest, reserve);
            volatile Tag a; Tag b; b.byte = a.byte;
            unsigned int length = s.length;
            void* begin = s.data;
            WString_ReplaceRange(dest, 0, 0, begin, (char*)begin + length * 2, &b.byte);
        }
    }
    // Release this key copy only; its mapped vector is owned by the section map.
    ~KeyValue() { if (key.flags >> 31) appFreeUni__FPv(key.data); }
    // Duplicate the key's storage and copy the mapped vector address.
    KeyValue(const KeyValue& source) {
        try {
        const String& s=source.key;
        String* dest=&key;
        if (!(s.flags >> 31)) {
            key.flags=s.flags;
            unsigned int b=*(volatile const unsigned int*)&s.length;
            void* c=*(void* const volatile*)&s.data;
            *(volatile unsigned int*)&key.length=b;
            *(void* volatile*)&key.data=c;
        } else {
            dest->flags=0;
            unsigned int reserve=s.length;
            dest->length=0; dest->data=0;
            WString_ReserveCapacity(dest,reserve);
            Tag b; volatile Tag a; b.byte=a.byte;
            unsigned int length=s.length;
            void* begin=s.data;
            WString_ReplaceRange(dest,0,0,begin,(char*)begin+length*2,&b.byte);
        }
        value=source.value;
        } catch(...) { __throw(0,0,0); }
    }
};
// Read a section key's length in UTF-16 code units.
inline unsigned int insertsize(String* key) {
    if (!(key->flags >> 31)) return *(unsigned char*)key & 127;
    return key->length;
}
// Insertion uses the same lexical ordering, with the compiler\'s original inlining shape.
inline int compareinsert(String* key, String* other) {
    unsigned int prefixLength,keyLengthCopy,otherLengthCopy,keyLength;
    unsigned int otherLength;
    const void* otherData;
    if (!(other->flags >> 31)) { otherLength = *(unsigned char*)other & 127; otherData = (char*)other + 2; }
    else { otherData = other->data; otherLength = other->length; }
    otherLengthCopy = otherLength;
    keyLength = !(key->flags >> 31) ? (*(unsigned char*)key & 127) : key->length;
    const void* keyData; unsigned int keyLengthSnapshot;
    if (!(key->flags >> 31)) { keyLengthSnapshot = *(unsigned char*)key & 127; keyData = (char*)key + 2; }
    else { keyData = key->data; keyLengthSnapshot = key->length; }
    keyLengthCopy = keyLengthSnapshot;
    prefixLength = minsize(keyLength,keyLengthCopy);
    int cmp = wmemcmp(keyData,otherData,minsize(otherLengthCopy,prefixLength));
    if (cmp == 0) {
        if (prefixLength < otherLength) return -1;
        else if (prefixLength == otherLength) return 0;
        else return 1;
    }
    return cmp;
}
#pragma opt_common_subs off
#pragma opt_propagation on
// Return the section\'s value slot, inserting a copied key with the default value if absent.
void** SaveBannerSectionMap_GetOrInsert(Tree* self, String* key) {
    Node* found;
    unsigned char left, absent;
    Node* parent = SaveBannerSectionTree_FindInsertionPosition(self, key, &found, &left, &absent);
    String* result;
    if (!found || (result = &found->key, compareinsert(result, key) < 0)) {
        KeyValue local(*key);
        local.value = s_bannerDefaultSectionValue;
        result = &SaveBannerSectionMap_InsertNode(self, parent, left, absent, &local)->key;
    }
    return (void**)(result + 1);
}

int SaveBanner_ClearLocalizedTextSections(Entries*);
extern const unsigned short* s_bannerTextLineDelimiter;
// UTF-16 language tags followed by the section-header format and closing delimiter.
struct BannerLanguageStringPool {
    unsigned short japanese[9], english[8], german[7], french[7], spanish[8];
    unsigned short italian[8], dutch[6], simplifiedChinese[13], traditionalChinese[13], korean[7];
    unsigned short alignmentPadding;
    unsigned short sectionHeaderFormat[6], closingBracket[2];
};
extern BannerLanguageStringPool languageTagStrings;
typedef char BannerLanguageStrings_header[((int)&((BannerLanguageStringPool*)0)->sectionHeaderFormat == 0xAE) ? 1 : -1];
unsigned int wcslen(const unsigned short*);
unsigned short* wcsstr(const unsigned short*, const unsigned short*);
void* Memory_Copy(void*, const void*, unsigned int);
int WideString_ScanFormatted(const unsigned short*, const void*, ...);
unsigned short* WString_FindSubstring(unsigned short*, const void*);
void WString_ConstructFromWChars(void*, const unsigned short*);
void* __nw__FUl(unsigned long);
void WStringVector_InitEmpty(void*);
void WStringVector_AppendCopy(Vec12*, String*);
struct TempString_u8029523C {
    unsigned int words[3];
    // Own a copied, terminated UTF-16 token for map insertion or vector append.
    TempString_u8029523C(const unsigned short* p) { WString_ConstructFromWChars(this, p); }
    // Release only this temporary's heap storage; containers retain their copies.
    ~TempString_u8029523C() { reinterpret_cast<WString*>(this)->~WString(); }
};
#pragma opt_common_subs off
enum { kBannerTextLineUnits = 128, kBannerSectionNameUnits = 32,
       kBannerLinesPerSection = 2, kBannerPrefixUnits = 1 };
// Split localized banner text into named sections, storing at most two text lines per section.
// The loaded buffer is borrowed. It is read as native-endian UTF-16, skipping one
// leading code unit (presumed BOM; never checked or swapped here). Delimiter/line
// lengths and pointer differences are code units; Memory_Copy's length is bytes.
// Size provenance: CSharedFilePtr_GetSize dispatches virtual slot 1. Its sync
// implementation returns fileSize from FileIO_GetFileSize, which also bounds the
// byte read (shared_file_loading.cpp); its async implementation returns CFileAsync_GetSize.
// The parser nevertheless compares that BYTE count to UTF-16 CODE-UNIT offsets.
// Preserve this existing mismatch for code identity. Text-format changes must not
// assume this is a correct unit limit; it can over-read unterminated/short input.
// wcsstr requires terminated input; section names must fit the 32-unit scan buffer.
// Returns 0 for absent/zero-size input, 1 after parsing; lines >=128 units are skipped.
int SaveBanner_ParseLocalizedTextSections(Entries* self) { unsigned short buffer[kBannerTextLineUnits];

    // Phase 1: clear owned sections, then borrow the text file and delimiter.
    unsigned short* base;
    if (!SaveBanner_ClearLocalizedTextSections(self)) return 0;
    base = (unsigned short*)SharedFileList_GetLoadedBufferAt(self, kBannerTextFile);
    if (base == 0) return 0;
    unsigned int legacyLimitInBytes;
    legacyLimitInBytes = (unsigned int)SharedFileList_GetFileSizeAt(self, kBannerTextFile);
    if (legacyLimitInBytes == 0) return 0;
    unsigned int delimiterUnits;
    unsigned short* current = base + kBannerPrefixUnits; unsigned short name[kBannerSectionNameUnits];
    delimiterUnits = wcslen(s_bannerTextLineDelimiter);

    Vec12* activeLines = 0;
    unsigned short* next = current - delimiterUnits;
    // Phase 2: tokenize one line at a time. A section owns its vector and copies of its
    // text lines; the local scan buffers/temporary strings are never retained.
    while (next && legacyLimitInBytes > (unsigned int)((current - base))) { unsigned int lineUnits;
        unsigned short* savedNext = (((delimiterUnits + next))); current = savedNext;
        next = wcsstr(current, s_bannerTextLineDelimiter);

        if (next) lineUnits = next - current;
        else lineUnits = legacyLimitInBytes - (current - base);
        if (lineUnits >= kBannerTextLineUnits) continue;
        unsigned int lineBytes = (lineUnits * 2);
        Memory_Copy(buffer, current, lineBytes);
        *(unsigned short*)((char*)buffer + lineBytes) = 0; // preserve the tokenizer\'s byte-count addressing
        // Phase 3: a new section owns a line vector; text lines are copied into it.
        if (WideString_ScanFormatted(buffer, languageTagStrings.sectionHeaderFormat, name)) {
            unsigned short* closingBracket = WString_FindSubstring(name, languageTagStrings.closingBracket);
            if (closingBracket) *closingBracket = 0;
            void* existing;
            { TempString_u8029523C temp(name); existing = *SaveBannerSectionMap_GetOrInsert((&((BannerStorage*)self)->textSections), (String*)&temp); }
            // Existing sections keep their original vector; a duplicate header
            // leaves activeLines unchanged, matching the original parser behavior.
            if (existing) continue;
            activeLines = (Vec12*)__nw__FUl(12);
            if (activeLines) WStringVector_InitEmpty(activeLines);
            if (activeLines == 0) continue;
            { TempString_u8029523C temp(name); *SaveBannerSectionMap_GetOrInsert((&((BannerStorage*)self)->textSections), (String*)&temp) = activeLines; }
        } else if (activeLines) {
            if (Container_ReadWordAtOffset4(activeLines) < kBannerLinesPerSection) {
                TempString_u8029523C temp(buffer);
                WStringVector_AppendCopy((Vec12*)activeLines, (String*)&temp);
            }
        }
    }
    return 1;
}

#pragma opt_common_subs off
#pragma opt_propagation on
extern char loadingUiContainerErrorStrings[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* __nwa__FUl(unsigned int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void* __dla__FPv(void*);
void __dt__Q23std12length_errorFv();
void __throw(void*, void*, void (*)()) __attribute__((noreturn));
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void RBTree_RebalanceAfterInsert(Node*, Node*);
struct ErrorBase_u8029523C {
    void* vt; char* data; void* ref;
    // Install the logic_error base interface before message construction.
    ErrorBase_u8029523C() { vt=__vt__Q23std11logic_error; }
    // Destroy the exception base without deleting the enclosing object.
    ~ErrorBase_u8029523C() { __dt__Q23std9exceptionFv(this, -1); }
};
struct BufferGuard_u8029523C {
    void* p; unsigned char* tag;
    // Guard error-message storage until shared ownership is established.
    BufferGuard_u8029523C(void* a, unsigned char* b):p(a),tag(b) {}
    // Free the message allocation if construction did not transfer it.
    ~BufferGuard_u8029523C() { if (p) __dla__FPv(p); }
};
struct RefCount {
    void* vt; int a,b; void* p;
    // Own the message with one initial strong and one control-block reference.
    RefCount(void* q) { a=1; b=1; vt=s_sharedCharDeleterVtable; p=q; }
};
struct VecExc { void* vt; char* text; void* rep; };
// Release exception text if constructing its shared control block fails.
struct VecGuard_u8029523C { char* text; char* allocator; ~VecGuard_u8029523C() { if (text) __dla__FPv(text); } };
// Retain this in-place throw expansion: an out-of-line helper changes exception cleanup code.
#define VEC_LENGTH_ERROR() { \
    VecExc e; \
    const char* message = loadingUiContainerErrorStrings; \
    VecExc* mp = &e; \
    mp->vt = __vt__Q23std11logic_error; \
    char alloc __attribute__((aligned(4))) = 0; \
    unsigned int len = strlen(message); \
    char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1)); \
    mp->text = text; \
    { \
        VecGuard_u8029523C guard = {text, &alloc}; \
        void* rep = new RefCount(text); \
        mp->rep = rep; \
        guard.text = 0; \
    } \
    strcpy(mp->text, message); \
    mp->vt = __vt__Q23std12length_error; \
    __throw(loadingUiContainerErrorStrings + 0x14, mp, __dt__Q23std12length_errorFv); \
}
// Throw a vector length_error, transferring its message to shared ownership.
inline void length_error_a() {
    VecExc e;
    VecExc* mp = &e;
    const char* message = loadingUiContainerErrorStrings;
    mp->vt = __vt__Q23std11logic_error;
    char alloc __attribute__((aligned(4))) = 0;
    unsigned int len = strlen(message);
    char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
    mp->text = text;
    {
        VecGuard_u8029523C guard = {text, &alloc};
        void* rep = new RefCount(text);
        mp->rep = rep;
        guard.text = 0;
    }
    strcpy(mp->text, message);
    mp->vt = __vt__Q23std12length_error;
    __throw(loadingUiContainerErrorStrings + 0x14, mp, __dt__Q23std12length_errorFv);
}
struct Error : ErrorBase_u8029523C {
    // Copy length-error text into shared storage, preserving volatile allocator slots.
    Error(const char* s) {
        Tag b; volatile Tag a; a.byte=0; b.byte=0;
        unsigned int n = strlen(s)+1;
        char* buffer = (char*)__nwa__FUl(n*2-n);
        data=buffer;
        { BufferGuard_u8029523C guard(buffer, &b.byte);
        ref = new RefCount(buffer);
        guard.p=0;
        }
        strcpy(data,s);
        vt=__vt__Q23std12length_error;
    }
    // Run the original error cleanup callback without deleting this stack object.
    ~Error() { __dt__Q33std6detail16char_array_guardFv(this,-1); }
};
struct ErrorB : ErrorBase_u8029523C {
    // Copy length-error text using this specialization's nonvolatile allocator slot.
    ErrorB(const char* s) {
        Tag b; b.byte=0;
        unsigned int n = strlen(s)+1;
        char* buffer = (char*)__nwa__FUl(n*2-n);
        data=buffer;
        { BufferGuard_u8029523C guard(buffer, &b.byte);
        ref = new RefCount(buffer);
        guard.p=0;
        }
        strcpy(data,s);
        vt=__vt__Q23std12length_error;
    }
    // Run the original error cleanup callback without deleting this stack object.
    ~ErrorB() { __dt__Q33std6detail16char_array_guardFv(this,-1); }
};
#pragma opt_propagation on
struct NodeGuard_u8029523C {
    Node** tree; Node* p;
    // Own a newly allocated section node until it is linked into the tree.
    NodeGuard_u8029523C(Node** t, Node* n):tree(t),p(n) {}
    // Free an unlinked node on insertion failure; linked nodes have p cleared.
    ~NodeGuard_u8029523C() { if(p) appFreeUni__FPv(p); }
};
struct ParentValue {
    Node* p;
    // Initialize the packed parent slot with the original construction/unwind shape.
    ParentValue(Node* n) { try { p=n; } catch(...) { __throw(0,0,0); } }
};
#pragma opt_propagation off
#pragma opt_common_subs off
// Allocate a section node, link it under its parent, and rebalance the red/black tree.
Node* SaveBannerSectionMap_InsertNode(Tree* self, Node* parent, int left, int first, void* input) {
    Node* node;
    register KeyValue* dest;
    if (self->count == 0xFFFFFFFF) {
        const char* text = loadingUiContainerErrorStrings+0x4B;
        const char* message = text;
        Error error(message);
        __throw(loadingUiContainerErrorStrings+0x14, &error, __dt__Q23std12length_errorFv);
    }
    Node* volatile a;
    Node* volatile b;
    volatile Tag tag; tag.byte=0;
    Node* allocated=(Node*)appMallocDummy__FUl(28);
    b=allocated;
    a=allocated;
    NodeGuard_u8029523C guard(&self->root,allocated);
    dest=(KeyValue*)&allocated->key;
    new(dest) KeyValue(*(KeyValue*)input);
    node=guard.p;
    node->right=0;
    guard.p=0;
    node->left=0;
    new(&node->parentAndColor) ParentValue(parent);
    if(left) parent->left=node; else parent->right=node;
    ++self->count;
    RBTree_RebalanceAfterInsert(node,self->root);
    if(first) self->first=node;
    return node;
}

#pragma opt_propagation on
#pragma opt_common_subs on
void WString_DestroyMapKey(Tree*, String*);
void SaveBanner_DestroyLocalizedTextSubtree(Tree*, Node*);
// Free a node's owned heap key; inline UTF-16 storage needs no separate release.
inline void cleanup_key(String* key) {
    if (key) {
        if (key) {
            if (key->flags >> 31) appFreeUni__FPv(key->data);
        }
    }
}
// Bounded postorder expansion: suffix L/R records the path below the entry root.
// Each helper handles left, right, owned key, then node. Depth-three helpers call
// the recursive entry for deeper children; they do not release line-vector values.
// Keep the distinct helper boundaries: they preserve inlining and unwind tables.
// Destroy the LLL subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728LLL(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy the LLR subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728LLR(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy LL in postorder through its two depth-three helpers.
inline void fn_80296728LL(Tree* self, Node* node) {
    if (node->left) fn_80296728LLL(self, node->left);
    if (node->right) fn_80296728LLR(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
// Destroy the LRL subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728LRL(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy the LRR subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728LRR(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy LR in postorder through its two depth-three helpers.
inline void fn_80296728LR(Tree* self, Node* node) {
    if (node->left) fn_80296728LRL(self, node->left);
    if (node->right) fn_80296728LRR(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
// Destroy the entry root's left subtree in postorder through LL and LR.
inline void fn_80296728L(Tree* self, Node* node) {
    if (node->left) fn_80296728LL(self, node->left);
    if (node->right) fn_80296728LR(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
// Destroy the RLL subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728RLL(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy the RLR subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728RLR(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy RL in postorder through its two depth-three helpers.
inline void fn_80296728RL(Tree* self, Node* node) {
    if (node->left) fn_80296728RLL(self, node->left);
    if (node->right) fn_80296728RLR(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
// Destroy the RRL subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728RRL(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy the RRR subtree's keys and nodes after recursively clearing descendants.
inline void fn_80296728RRR(Tree* self, Node* node) {
    if (node->left) SaveBanner_DestroyLocalizedTextSubtree(self, node->left);
    if (node->right) SaveBanner_DestroyLocalizedTextSubtree(self, node->right);
    WString_DestroyMapKey(self, &node->key);
    appFreeUni__FPv(node);
}
// Destroy RR in postorder through its two depth-three helpers.
inline void fn_80296728RR(Tree* self, Node* node) {
    if (node->left) fn_80296728RRL(self, node->left);
    if (node->right) fn_80296728RRR(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
// Destroy the entry root's right subtree in postorder through RL and RR.
inline void fn_80296728R(Tree* self, Node* node) {
    if (node->left) fn_80296728RL(self, node->left);
    if (node->right) fn_80296728RR(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
// Destroy section keys and tree nodes in postorder; values are released separately.
void SaveBanner_DestroyLocalizedTextSubtree(Tree* self, Node* node) {
    if (node->left) fn_80296728L(self, node->left);
    if (node->right) fn_80296728R(self, node->right);
    cleanup_key(&node->key);
    appFreeUni__FPv(node);
}
struct CleanupArray { String* data; int count; int capacity; };
// Release a fringe node's descendants, then its owned key and node allocation (values are gone).
static inline void ClearBannerFringe(Tree* tree, Node* leaf) {
    if (leaf->left) SaveBanner_DestroyLocalizedTextSubtree(tree, leaf->left);
    if (leaf->right) SaveBanner_DestroyLocalizedTextSubtree(tree, leaf->right);
    cleanup_key(&leaf->key);
    appFreeUni__FPv(leaf);
}

// Release one expanded branch in postorder; no line-vector ownership remains at this stage.
static inline void ClearBannerBranch(Tree* tree, Node* branch) {
    Node* leaf = branch->left;
    if (leaf) ClearBannerFringe(tree, leaf);
    leaf = branch->right;
    if (leaf) ClearBannerFringe(tree, leaf);
    cleanup_key(&branch->key);
    appFreeUni__FPv(branch);
}

// Release both branches below an expanded child before deleting that child.
static inline void ClearBannerChild(Tree* tree, Node* child) {
    Node* branch = child->left;
    if (branch) ClearBannerBranch(tree, branch);
    branch = child->right;
    if (branch) ClearBannerBranch(tree, branch);
    cleanup_key(&child->key);
    appFreeUni__FPv(child);
}

#pragma opt_common_subs off
#pragma opt_propagation on
// Release each section\'s line vector, then destroy all keys and reset the tree sentinel.
int SaveBanner_ClearLocalizedTextSections(Entries* self) {
    Node* root;
    Node* child;
    {
        unsigned int remainingLines;
        String* lineEnd;
        // Phase 1: in-order walk releases line strings, their vector storage and
        // wrappers, but leaves keys/nodes intact so traversal links remain valid.
        Node* node = ((BannerStorage*)self)->textSections.first;
        CleanupArray* lineVector;
        Node* sentinel = (Node*)&((BannerStorage*)self)->textSections.root;
        while (sentinel != node) {
            lineVector = node->lines;
            if (lineVector) {
                if (lineVector) {
                    if (lineVector) {
                        if (lineVector) {
                            if (lineVector) {
                                String* begin = lineVector->data;
                                if (begin) {
                                    remainingLines = lineVector->count;
                                    lineEnd = begin + remainingLines;
                                    lineVector->count -= remainingLines;
                                    while (remainingLines) {
                                        lineEnd -= 1;
                                        if (lineEnd) {
                                            if (lineEnd->flags >> 31) appFreeUni__FPv(lineEnd->data);
                                        }
                                        remainingLines = remainingLines - 1;
                                    }
                                    appFreeUni__FPv(lineVector->data);
                                }
                            }
                        }
                    }
                    __dl__FPv(lineVector);
                }
                node->lines = (CleanupArray*)(0);
            }
            Node* next = node->right;
            if (next) {
                Node* left;
                while ((left = next->left) != 0) next = left;
                node = next;
            }
            else {
                Node* parent;
                while (node != (parent = (Node*)(node->parentAndColor & ~1U))->left) node = parent;
                node = parent;
            }
        }
    }
    // Phase 2: postorder erasure releases keys/nodes, then resets the empty-tree
    // count/root/first sentinel. Values were already released in phase 1.
    root = ((&((BannerStorage*)self)->textSections))->root;
    if (root) {
        child = root->left;
        if (child) ClearBannerChild(&((BannerStorage*)self)->textSections, child);
        Node* savedRight = root->right;
        child = savedRight;
        if (child) ClearBannerChild(&((BannerStorage*)self)->textSections, child);
        cleanup_key(&root->key);
        appFreeUni__FPv(root);
        ((&((BannerStorage*)self)->textSections))->count = 0;
        ((&((BannerStorage*)self)->textSections))->root = 0;
        ((BannerStorage*)self)->textSections.first = (Node*)&((BannerStorage*)self)->textSections.root;
    }
    return 1;
}

struct ArrayGuard4 {
    void** data; unsigned int count, capacity; unsigned int* owner; unsigned int offset;
    // Start a temporary allocation for file-handle vector growth.
    ArrayGuard4(unsigned int* p):data(0),count(0),capacity(0),owner(p),offset(0) {}
    // Free storage only; transfer code clears old slots before this runs.
    ~ArrayGuard4() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Choose the larger increment for a vector of four-byte file-handle slots.
inline const unsigned int& maxgrow4(const unsigned int& a,const unsigned int& b) { const unsigned int* r = &a; if (a < b) r = &b; return *r; }
// Fill n file-handle slots; callers use null while resizing before loading.
static inline void fill4(void** out, unsigned int n, void* value) { for (; n; --n) *out++ = value; }
#pragma scheduling on
#pragma opt_loop_invariants off
#pragma opt_common_subs off
#pragma opt_propagation off
// Release old file handles and start loading the text, icon and banner files in order.
int CBanner_ReloadFileHandles(Entries* self) {
    // Phase 1: release every old shared-file handle, retaining vector capacity.
    void* g[3];
    void** p = self->entries;
    while (self->count + self->entries != p) {
        void* e = *p;
        if (e) {
            CSharedFileHandle_Destroy(e, 1);
            *p = 0;
        }
        ++p;
    }
    {

        self->count = (self->count) - self->count;
    }
    void* zero = 0;
    // Phase 2: resize to three null slots. Reserve/copy/clear/swap paths join
    // before any file is opened; temporary guards own abandoned allocations.
    do {
        if (3 > self->count) { unsigned int grow; unsigned int capv = self->capacity;
            unsigned int cap;

            unsigned int n = 3 - self->count;
            if (n > capv || self->count > capv - n) {
                unsigned int unused;
                cap = self->capacity;
                if ((self->count + n) - capv > 0x3FFFFFFF - cap) {
                    length_error_a();
                }
                if (cap < 0x15555555u) {
                    unused = ((cap + 1)) * 3 / 5;
                } else if (cap < 0x2AAAAAAAu) {
                    unused = ((cap + 1)) >> 1;
                }
            } else {
                void** dest = (self->count + self->entries);
                fill4(dest, n, zero);
                self->count = (self->count + n);
                break;
            }

            ArrayGuard4 tmp(&self->capacity);
            unsigned int need = self->count + n - self->capacity;
            cap = self->capacity;
            if (need > 0x3FFFFFFF - cap) {
                VEC_LENGTH_ERROR();
            }
            unsigned int grownB;
            if (cap < 0x15555555u) {
                grow = (cap + 1) * 3 / 5;
                unsigned int grownA; unsigned int savedGrow = grow; grownA = savedGrow;
                cap = cap + maxgrow4(grownA, need);
            } else if (!(!(cap < 0x2AAAAAAAu))) {
                grow = (cap + 1) >> 1;
                grownB = grow;
                cap = cap + maxgrow4(grownB, need);
            } else {
                cap = 0x3FFFFFFF;
            }
            if (cap > 0x3FFFFFFFu) {
                VEC_LENGTH_ERROR();
            }
            tmp.data = (void**)appMallocDummy__FUl(cap * 4);
            tmp.capacity = cap;
            int oldSize = self->count;
            tmp.offset = oldSize;
            fill4(tmp.data + oldSize + tmp.count, n, zero);

            tmp.count = tmp.count + n;
            unsigned int moved;
            void** begin = self->entries;
            moved = (unsigned int)((self->count + begin) - begin);

            tmp.offset -= moved; int bytes = moved * 4;
            memcpy(tmp.data + tmp.offset, begin, bytes);
            memset(begin, 0, bytes);
            tmp.count += moved;

            void** oldData = self->entries;
            unsigned int savedCapacityCopy = self->capacity; unsigned int savedCapacity = tmp.capacity;

            void** newData = tmp.data;
            self->count = 0;
            self->capacity = savedCapacity;
            tmp.data = oldData;
            int oldCapacity = savedCapacityCopy; int savedOldCapacity = oldCapacity; tmp.capacity = savedOldCapacity;
            self->entries = newData;
            self->count = (tmp.count);
            tmp.count = 0;
        } else if (3 < self->count) {
            self->count = self->count - (self->size() - 3);
        }
    } while (0);
    // Phase 3: create handles in text/icon/banner order. Failure returns 0 with
    // earlier handles retained for the next cleanup; success does not mean loaded.
    g[0] = s_bannerCommentFilename; g[1] = s_bannerIconFilename; g[2] = s_bannerImageFilename;
    void** it = self->entries;
    while (self->count + self->entries != it) {
        void* e = SharedFile_Create(g[it - self->entries], 0);
        *it = e;
        if (e == 0) return 0;
        ++it;
    }
    return 1;
}
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
struct CopyString : String {
    // Copy inline UTF-16 bytes directly or allocate an independent heap buffer.
    CopyString(const String& s) {
        try {
            if (!(s.flags >> 31)) {
                flags=s.flags;
                unsigned int b=*(volatile const unsigned int*)&s.length;
                void* c=*(void* const volatile*)&s.data;
                *(volatile unsigned int*)&length=b;
                *(void* volatile*)&data=c;
            } else {
                flags=0;
                unsigned int reserve=s.length;
                length=0; data=0;
                WString_ReserveCapacity(this,reserve);
                Tag b; volatile Tag a; b.byte=a.byte;
                unsigned int n=s.length;
                void* begin=s.data;
                WString_ReplaceRange(this,0,0,begin,(char*)begin+n*2,&b.byte);
            }
        } catch (...) { __throw(0,0,0); }
    }
};
struct ArrayGuard12 {
    String* data; unsigned int count, capacity; unsigned int* owner; unsigned int offset;
    // Track a constructed string range during growth, beginning at offset.
    ArrayGuard12(unsigned int* p):data(0),count(0),capacity(0),owner(p),offset(0) {}
    // Destroy live owned strings, then release their enclosing allocation.
    ~ArrayGuard12() {
        if(data) {
            unsigned int n=count;
            String* end=data+n;
            count-=n;
            volatile Tag tag; tag.byte=0;
            while(n!=0) { --end; if(end && (end->flags>>31)) appFreeUni__FPv(end->data); --n; }
            appFreeUni__FPv(data);
        }
    }
};
// Choose the larger increment for a vector of 12-byte owned wide strings.
inline const unsigned int& maxgrow12(const unsigned int& a,const unsigned int& b) { return a<b ? b:a; }
#pragma scheduling on
#pragma opt_loop_invariants off
#pragma opt_common_subs off
#pragma opt_propagation off
// Append a copied wide string, growing and transferring the buffer with unwind cleanup.
void WStringVector_AppendCopy(Vec12* self, String* input) {
    volatile unsigned char entrytag;
    entrytag=0; register String* dest;
    if (self->count < self->capacity) {
        // Spare capacity: copy the input string before publishing the count.
        dest = (String*)self->entries + self->count;
        new(dest) CopyString(*input);
        self->count += 1;
    } else {
        // Growth: bound the number of 12-byte entries, then allocate a larger buffer.
        volatile unsigned char growtag; growtag=0;
        volatile unsigned char checktag; checktag=0;
        if (0x15555555 - self->capacity < 1) {
            const char* text=loadingUiContainerErrorStrings;
            const char* message=text;
            Error error(message);
            __throw(loadingUiContainerErrorStrings + 0x14, &error, __dt__Q23std12length_errorFv);
        }
        ArrayGuard12 guard(&self->capacity);
        const unsigned int delta = (1 + self->count) - self->capacity;
         register unsigned int newcap;
        register unsigned int oldcap = self->capacity;
        if (delta > 0x15555555 - oldcap) {
            const char* text=loadingUiContainerErrorStrings;
            const char* message=text;
            Error error(message);
            __throw(loadingUiContainerErrorStrings + 0x14, &error, __dt__Q23std12length_errorFv);
        }
        if (oldcap < 0x071C71C7) {
            unsigned int candidate = (3 * (1 + oldcap))/5;
            newcap = oldcap + maxgrow12(candidate,delta);
        } else if (oldcap < 0x0E38E38E) {
            unsigned int candidate = (((1 + oldcap))) >> 1;
            newcap = oldcap + maxgrow12(candidate,delta);
        } else {
            newcap = 0x15555555;
        }
        volatile Tag allocTag; allocTag.byte=0;
        if(newcap>0x15555555) {
            const char* text=loadingUiContainerErrorStrings;
            const char* message=text;
            Error error(message);
            __throw(loadingUiContainerErrorStrings+0x14,&error,__dt__Q23std12length_errorFv);
        }
        String* newbuf=(String*)appMallocDummy__FUl((newcap*12));
        unsigned int insertion=self->count;
        guard.data=newbuf; guard.capacity=newcap; guard.offset=insertion;

        // Construct the appended copy first; transfer existing string objects by
        // bytes and clear old slots so their heap pointers have only one owner.
        volatile Tag copytag; copytag.byte=0; dest=newbuf+insertion+guard.count;
        for(int n=1;0!=n;--n,++dest) { new(dest) CopyString(*input); guard.count += 1; }
        volatile Tag movetag;
        movetag.byte=0;
        String* begin=(String*)self->entries; int n=((String*)self->entries+self->count)-begin;
        guard.offset-=n;
        register unsigned int bytes=n*12;
        memcpy(guard.data+guard.offset,begin,bytes);
        memset(begin,0,bytes);
        guard.count+=n;
        unsigned int newcount=guard.count;
        int oldcount=self->count;
        String* oldbuf=(String*)self->entries;
        int oldcapacity=self->capacity;
        unsigned int savedOffset = guard.offset; String* first=((oldbuf+savedOffset));
         String* end=oldcount+first;
        // Publish the new allocation; guard receives old storage for cleanup.
        self->capacity=guard.capacity; self->entries=guard.data;
        guard.capacity=oldcapacity; guard.data=oldbuf;
        self->count=newcount; guard.count=oldcount;
        volatile Tag cleartag; cleartag.byte=0;
        while(first<end) { --end; if(end && (end->flags>>31)) appFreeUni__FPv(end->data); }
        guard.count=0;
    }
}
} // extern "C"


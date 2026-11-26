// Packed string-pool view; byte extents come from the globals research.
struct CaddieStrings {
    char padding0[32];
    char shadowGroup[7];
    char padding27[4];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char dialogueExtension[5];
};
enum {
    kCaddieStringsShadowGroupOffset = (int)&((CaddieStrings*)0)->shadowGroup,
    kCaddieStringsVectorLengthErrorOffset = (int)&((CaddieStrings*)0)->vectorLengthError,
    kCaddieStringsLengthErrorTypeOffset = (int)&((CaddieStrings*)0)->lengthErrorType,
    kCaddieStringsDialogueExtensionOffset = (int)&((CaddieStrings*)0)->dialogueExtension,
};
#include "game/string.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CGolfCaddie.h"
#include "classes/GolfCaddieEntryView.h"
// Borrow the shared-document layout without pointer adjustment.
static inline CSharedDoc* AsCSharedDoc(void* object) {
    return (CSharedDoc*)object;
}
// Borrow the golf-document layout without pointer adjustment.
static inline CGolfDoc* AsCGolfDoc(void* object) {
    return (CGolfDoc*)object;
}

#include "sdk_protos.h"
#include "classes/CPetBody.h"
#include "classes/CConteCaddie.h"
#include "classes/CShadowManager.h"

extern "C" {

// Entry ownership: the caddie owns a vector of entry pointers, including null
// entries for missing IFF records. Each real entry owns two dialogue trees.
// Tree header = count/root/first/end; end points at the embedded root slot.
// Nodes = left/right/tagged-parent at +0/+4/+8, key/value pair begins at +0x0C.
// Narrow key and wide text independently own heap data when flags bit 31 is set.
// Temporary resource handles and loaded file bytes are released after parsing.
struct CaddieTreeHeader {
    unsigned int count;
    void* root;
    void* first;
    void* end;
};
struct CaddieTreeNode {
    void* left;
    void* right;
    unsigned int parentAndColor;
    char pair[1];
};
struct CaddieDialogueStorage {
    char padding[0x18];
    CaddieTreeHeader dialogue;
};
struct DialogueTableHeader {
    unsigned int words[2];
    int entryCount;
};

// Navigable entry model: the Fn* views below are subsets of this single allocation.
// Both tree headers describe owned nodes and strings; pointer fields retain the existing ABI.
struct CaddieEntryStorageView {
    void* petBody;
    char unresolved04[0x0c];
    IFF_STRUCT::sCaddie* caddieRecord;
    void* instanceData;
    CaddieTreeHeader dialogue;
    CaddieTreeHeader secondaryDialogue;
    char unresolved38[0x0c];
    unsigned char ringEnabled, caddieViewEnabled;
    char alignment46[2];
};
typedef char CaddieEntryStorage_size[sizeof(CaddieEntryStorageView) == 0x48 ? 1 : -1];
// Borrow the primary dialogue tree embedded at +0x18 in the owned entry.
static inline CaddieTreeHeader* PrimaryCaddieDialogue(void* entry) {
    return &((CaddieEntryStorageView*)entry)->dialogue;
}

extern char __vt__12CConteCaddie[];
extern char golfCaddieStrings[];
extern void* g_sharedDoc;
extern void* g_shadowManager;
extern void* g_soundManager;
extern const float kFloat_0_GolfCaddie;

void* __nw__FUl(unsigned int);
extern "C" void ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(void*, const char*, void*);
void GfxObjList_RemoveGroupByName(void*, const char*);
void ShadowModel_SetNamedProjectionEnabled(void*, const char*, int);
void* SharedDoc_GetItemInventory(void);
void* ItemInventory_FindCaddieData(void*, void*);
void CaddieDialogue_LoadTextMap(void*, void*);
void CGolfCaddie_LoadAnimationPrerequisites(void*, void*);
NString Storage_ClearThreeWords();
int Sound_BuildCaddieBankName(void*, void*);
const char* MWString_GetCString(void*);
void SoundManager_AcquireSoundBank(void*, const char*);
void StringVector_AppendCopy(void*, void*);

struct CaddieEntryVector;
void CaddieEntryList_Append(CaddieEntryVector*, void* const*);

// Local view names are retained because compiler-generated linkage refers to them.
// Reset-only flags/timers below still need consumer evidence before stronger naming.
// Reset view of CGolfCaddie: +0x1C active entry, +0x20 entry vector, +0x24 count;
// +0x30 animation state; +0x40 position-Z bits, +0x44 movement delay,
// +0x48 yaw, +0x58/+0x59 flags,
// +0x60 counter, +0x64 flag, +0x68 timer. Reset values alone do not establish
// timer units or flag meanings; retain the existing tentative field names.
struct CGolfCaddieView {
    char pad00[0x1C];
    void* activeCaddie;
    void** entries;
    unsigned int entryCount;
    char pad28[8];
    unsigned int animationState;
    char pad34[0xC];
    unsigned int positionZBits;
    float movementDelay;
    float yaw;
    char pad4C[0x0C];
    unsigned char movementFlag;
    unsigned char visible;
    char pad5A[6];
    unsigned int movementCounter;
    unsigned char messageFlag;
    char pad65[3];
    float movementTimer;
};

struct NStringView {
    unsigned int flags;
    unsigned int length;
    const char* data;
};
struct Fn80128E44Object;
struct Fn8012899CObject;
void GolfCaddieEntry_Init(Fn8012899CObject*);

// Select the current player's caddie, update its named shadow, and reset movement state.
void CGolfCaddie::OnInit() {
    CGolfCaddieView* self = (CGolfCaddieView*)this;
    int index = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
    void* item;
    if (index < 0 || (unsigned int)index >= self->entryCount)
        item = 0;
    else
        item = self->entries[index];
    self->activeCaddie = item;
    if (item != 0 && *(void**)item != 0) {
        void* manager = g_shadowManager;
        void* model = ((CPetBody*)(*(void**)item))->GetPuppet();
        ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
            manager, golfCaddieStrings + kCaddieStringsShadowGroupOffset, model);
    } else
        GfxObjList_RemoveGroupByName(g_shadowManager, golfCaddieStrings + kCaddieStringsShadowGroupOffset);
    self->visible = 1;
    self->yaw = kFloat_0_GolfCaddie;
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, golfCaddieStrings + kCaddieStringsShadowGroupOffset, 1);
    self->animationState = 0;
    self->movementTimer = kFloat_0_GolfCaddie;
    self->movementDelay = kFloat_0_GolfCaddie;
    self->movementFlag = 0;
    self->messageFlag = 0;
    self->movementCounter = 0;
}

// Borrow the smaller key length for bounded byte comparison.
static inline const unsigned int& tree_min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

#pragma opt_loop_invariants off
// The high flag bit selects heap storage; short strings start after the length byte.
static inline const char* tree_data(const NStringView& s, unsigned int flags, unsigned int& length) {
    unsigned int n;
    const char* p;
    if (!(flags >> 31)) {
        p = (const char*)&s + 1;
        n = *(const unsigned char*)&s & 0x7F;
    } else {
        p = s.data;
        n = s.length;
    }
    length = n;
    return p;
}
// Compare byte prefixes and then lengths, preserving the original inline-string ABI.
static inline int tree_compare(const NStringView& a, const NStringView& b) {
    unsigned int len, nlen, requested, vlen;
    unsigned int klen;
    const char* kdata;
    if (!(b.flags >> 31)) {
        kdata = (const char*)&b + 1;
        klen = *(const unsigned char*)&b & 0x7F;
    } else {
        kdata = b.data;
        klen = b.length;
    }
    unsigned int aflags = a.flags;
    requested = klen;
    unsigned int temp = (!(aflags >> 31)) ? *(const unsigned char*)&a & 0x7F : a.length;
    unsigned int nextflags = a.flags;
    vlen = temp;
    const char* ndata;
    ndata = tree_data(a, nextflags, nlen);
    len = tree_min(vlen, nlen);
    int cmp = memcmp(ndata, kdata, tree_min(requested, len));
    if (cmp == 0) {
        if (len < klen)
            return -1;
        if (len == klen)
            return 0;
        return 1;
    }
    return cmp;
}

#pragma use_lmw_stmw on
// Find an insertion parent and the last encountered node with key <= the requested key.
// Outputs: insertLeft selects the child link; noPredecessor stays true until a
// right descent. Empty trees return the embedded root slot as the insertion parent.
void* CaddieDialogueTextMap_FindInsertionPosition(void* self, NStringView* key, void** predecessor,
                                                  unsigned char* insertLeft, unsigned char* noPredecessor) {
    void* insertionParent;
    *predecessor = 0;
    void* node = ((CaddieTreeHeader*)self)->root;
    *insertLeft = 1;
    *noPredecessor = 1;
    insertionParent = &((CaddieTreeHeader*)self)->root;
    while (node != 0) {
        insertionParent = node;
        int cmp = tree_compare(*key, *(NStringView*)((CaddieTreeNode*)node)->pair);
        if (cmp < 0) {
            node = *(void**)node;
            *insertLeft = 1;
        } else {
            *predecessor = node;
            node = ((CaddieTreeNode*)node)->right;
            *insertLeft = 0;
            *noPredecessor = 0;
        }
    }
    return insertionParent;
}
#pragma use_lmw_stmw reset
#pragma opt_loop_invariants reset

#pragma use_lmw_stmw on
// Create a caddie entry, or return the existing one when reuseExisting is set.
// The new entry loads its dialogue text map and animation prerequisite map,
// acquires the sound bank named after its model, then joins the player-order
// list. A missing IFF record appends a null entry in its place.
void* CGolfCaddie::CreateCaddieEntry(unsigned int caddieTypeId, unsigned int instanceId, int reuseExisting,
                                     int createPetBody) {
    void* result;
    if (reuseExisting != 0) {
        result = FindExistingEntry(caddieTypeId, instanceId);
        if (result != 0)
            return result;
    }
    IFF_STRUCT::sCaddie* caddieRecord =
        (IFF_STRUCT::sCaddie*)ItemInventory_FindCaddieData(SharedDoc_GetItemInventory(), (void*)caddieTypeId);
    if (caddieRecord != 0) {
        void* created = __nw__FUl((unsigned int)0x48);
        if (created != 0)
            GolfCaddieEntry_Init((Fn8012899CObject*)created);
        result = created;
        AsGolfCaddieEntry80128808(result)->ringEnabled = 0;
        AsGolfCaddieEntry80128808(result)->caddieViewEnabled = 1;
        AsGolfCaddieEntry80128808(result)->caddieRecord = caddieRecord;
        if (createPetBody != 0)
            CreatePetBodyForEntry(result, (void*)instanceId);
        CaddieDialogue_LoadTextMap(this, result);
        CGolfCaddie_LoadAnimationPrerequisites(this, result);
        if (result != 0 && AsGolfCaddieEntry80128808(result)->caddieRecord != 0 &&
            AsGolfCaddieEntry80128808(result)->caddieRecord->model != 0) {
            NString local = Storage_ClearThreeWords();
            if (Sound_BuildCaddieBankName(AsGolfCaddieEntry80128808(result)->caddieRecord->model, &local)) {
                const char* name = MWString_GetCString(&local);
                SoundManager_AcquireSoundBank(g_soundManager, name);
                StringVector_AppendCopy(&soundBankNames, &local);
            }
        }
        CaddieEntryList_Append((CaddieEntryVector*)&caddieEntries, &result);
    } else {
        void* empty = 0;
        CaddieEntryList_Append((CaddieEntryVector*)&caddieEntries, &empty);
    }
    return result;
}
#pragma use_lmw_stmw reset

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

struct Fn80129A1CObject {
    void* prefix;
    unsigned int stringFlags;
    char pad08[4];
    void* stringData;
};

struct CaddieTreeNodePairView {
    unsigned int keyFlags;
    char pad04[4];
    void* keyData;
    unsigned int valueType;
    unsigned int valueFlags;
    char pad14[4];
    void* valueData;
};

struct Fn80128E44Item {
    char pad00[0x10];
    void* caddieRecord;
    void* instanceData;
};

struct Fn80128E44Detail {
    char pad00[8];
    unsigned int instanceId;
};
struct Fn80128E44State {
    char pad00[0x50];
    unsigned int typeId;
};
struct Fn80128E44Owner {
    char pad00[0x14];
    Fn80128E44State* state;
};
struct Fn80128E44Object {
    char pad00[0x20];
    Fn80128E44Item** entries;
    unsigned int entryCount;
};

struct Fn80128808Object {
    void* vtable;
    void* statePointerA;
    void* statePointerB;
};

// Dialogue trees are headers embedded in each caddie entry, rather than shared globals.
struct Fn8012899CObject {
    void* petBody;
    char pad04[0x0C];
    void* caddieRecord;
    void* instanceData;
    void* dialogueCount;
    void* dialogueRoot;
    char pad20[4];
    void* dialogueEnd;
    void* secondaryDialogueCount;
    void* secondaryDialogueRoot;
    char pad30[4];
    void* secondaryDialogueEnd;
};

// Navigating the recovered views: CGolfCaddie owns the entry vector at +0x20;
// each entry has petBody +0, IFF record +0x10, instance +0x14, dialogue headers
// +0x18/+0x28, and state bytes +0x44/+0x45. Fn* views expose subsets of this
// same allocation; none is another owned object. Narrow/wide strings below use
// identical 12-byte headers but different inline starts (+1 byte / +2 bytes).

// Install the story-caddie vtable and clear its two pointer slots.
CConteCaddie::CConteCaddie() {
    Fn80128808Object* self = (Fn80128808Object*)this;
    void* vtable = __vt__12CConteCaddie;
    void* zero = 0;
    self->statePointerA = zero;
    self->vtable = vtable;
    self->statePointerB = zero;
}

struct Fn80128824Dispatch {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
};

// Dispatch the caddie's virtual initialization hook after reload.
void CGolfCaddie::OnReload() {
    ((Fn80128824Dispatch*)this)->slot3();
}

// Initialize an empty caddie entry and both dialogue-tree sentinels.
void GolfCaddieEntry_Init(Fn8012899CObject* self) {
    self->petBody = 0;
    self->caddieRecord = 0;
    self->instanceData = 0;
    self->dialogueCount = 0;
    self->dialogueRoot = 0;
    self->dialogueEnd = &self->dialogueRoot;
    self->secondaryDialogueCount = 0;
    self->secondaryDialogueRoot = 0;
    self->secondaryDialogueEnd = &self->secondaryDialogueRoot;
}

// Release the owned long string in this dialogue wrapper, then optionally free the wrapper.
void* dtor_80129A1C(Fn80129A1CObject* self, int del) {
    if (self != 0) {
        if (&self->stringFlags != 0 && self->stringFlags >> 31)
            appFreeUni__FPv(self->stringData);
        if (del > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the narrow dialogue key and wide value, then optionally free the pair.
void* CaddieTreeNodePair_dtor(CaddieTreeNodePairView* self, int del) {
    if (self != 0) {
        // Keep the original address-wrap checks; offsets derive from the pair layout.
        if ((char*)self + (int)&((CaddieTreeNodePairView*)0)->valueType != 0) {
            if ((char*)self + (int)&((CaddieTreeNodePairView*)0)->valueFlags != 0 && (self->valueFlags >> 31))
                appFreeUni__FPv(self->valueData);
        }
        if (self != 0) {
            if (self->keyFlags >> 31)
                appFreeUni__FPv(self->keyData);
        }
        if (del > 0)
            __dl__FPv(self);
    }
    return self;
}

struct CaddiePredicate {
    unsigned int key, value;
    // Capture the required type and the optional instance identity.
    CaddiePredicate(unsigned int k, unsigned int v) : key(k), value(v) {}
    // Match type alone without instanceData, otherwise require both IDs.
    bool operator()(Fn80128E44Item* item) const {
        if (item == 0)
            return false;
        if (item->instanceData == 0)
            return ((IFF_STRUCT::sCaddie*)item->caddieRecord)->common.typeId == key;
        return ((Fn80128E44Detail*)item->instanceData)->instanceId == value &&
               ((IFF_STRUCT::sCaddie*)item->caddieRecord)->common.typeId == key;
    }
};
// Return the first entry satisfying the type/instance predicate, or the end iterator.
static inline Fn80128E44Item** caddie_find(Fn80128E44Item** current, Fn80128E44Item** end, CaddiePredicate pred) {
    while (current != end && !pred(*current))
        ++current;
    return current;
}
// Find a caddie with the requested type and optional instance record.
void* CGolfCaddie::FindExistingEntry(unsigned int key, unsigned int value) {
    Fn80128E44Object* self = (Fn80128E44Object*)this;
    if (key == 0)
        return 0;
    Fn80128E44Item** current = self->entries;
    Fn80128E44Item** end = current + self->entryCount;
    current = caddie_find(current, end, CaddiePredicate(key, value));
    return current != end ? *current : 0;
}

struct CaddieEntryVector {
    void** first;
    unsigned int size;
    unsigned int capacity;
};

void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void __dla__FPv(void*);
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv();
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

// ---- std::vector scaffolding (inlined library code, not game logic) ----
// Everything from here to CaddieEntryList_Append is the Metrowerks std::vector<void*> grow path with its
// "vector length error" exception: VectorErrorRef/VectorErrorGuard/VectorErrorBase build the std::length_error
// message copy, vector_length_error() throws it, VectorGrow is the temporary replacement array.
// Game code only needs to know: CaddieEntryList_Append(vec, &ptr) is vec.push_back(ptr).
struct VectorErrorRef {
    void* table;
    unsigned int count, count2;
    char* text;
    // Adopt an exception message into its shared deleter record.
    VectorErrorRef(char* p) {
        count = 1;
        count2 = 1;
        table = s_sharedCharDeleterVtable;
        text = p;
    }
};
struct VectorErrorGuard {
    char* ptr;
    char* allocator;
    // Arm cleanup until the message control block adopts the allocation.
    VectorErrorGuard(char* p, char* a) : ptr(p), allocator(a) {}
    // Delete only an uncommitted temporary character array.
    ~VectorErrorGuard() {
        if (ptr)
            delete[] ptr;
    }
};
// std::exception/logic_error base. length_error reuses it: the table is first set to logic_error's, then to length_error's.
struct VectorException {
    void* table;
    // Install the logic_error base table for staged exception construction.
    VectorException() {
        table = __vt__Q23std11logic_error;
    }
    // Delegate exception-base destruction to the existing ABI wrapper.
    ~VectorException() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
struct VectorErrorBase : VectorException {
    char* text;
    VectorErrorRef* ref;
    // Copy a NUL-terminated error message and transfer buffer ownership to ref.
    void initialize(const char* s, char* a) {
        table = __vt__Q23std11logic_error;
        unsigned int len = strlen(s) + 1;
        char* buffer = (char*)__nwa__FUl(2 * len - len);
        text = buffer;
        {
            VectorErrorGuard guard(buffer, a);
            ref = new VectorErrorRef(buffer);
            guard.ptr = 0;
        }
        strcpy(text, s);
    }
};
// Throw the caddie-entry vector length exception using its original string allocation ABI.
static inline void vector_length_error() {
    const char* message;
    VectorErrorBase* error;
    message = golfCaddieStrings + kCaddieStringsVectorLengthErrorOffset;
    // Initialize the opaque exception-string allocator token to zero.
    struct Allocator {
        char c;
        char pad[3];
        Allocator() : c(0) {}
    } allocator;
    char storage[sizeof(VectorErrorBase)];
    error = (VectorErrorBase*)storage;
    error->initialize(message, &allocator.c);
    error->table = __vt__Q23std12length_error;
    __throw(golfCaddieStrings + kCaddieStringsLengthErrorTypeOffset, error, (void*)__dt__Q23std12length_errorFv);
}
// Pointer-vector limits: max element count is 0xFFFFFFFF / sizeof(void*); growth is x1.6 below one third of
// the maximum, x1.5 below two thirds, otherwise the maximum.
enum {
    kVecMaxPointers = 0x3FFFFFFF,
    kVecGrowThreshold1 = 0x15555555,
    kVecGrowThreshold2 = 0x2AAAAAAA
};
struct VectorGrow {
    void** first;
    unsigned int size, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Guard an empty replacement pointer array until the vector swap commits.
    VectorGrow(volatile unsigned int* p) : first(0), size(0), capacity(0), owner((unsigned int*)p), offset(0) {}
    // Release the old array after a swap, or a failed replacement on unwind.
    ~VectorGrow() {
        if (first) {
            size = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Borrow the larger pointer-capacity requirement.
static inline const unsigned int& vector_max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

#pragma use_lmw_stmw on
// Append a caddie pointer, growing and transferring the old vector storage when full.
// Vector stores pointers only: the old pointer array is freed after a swap;
// pointed-to entries are neither cloned nor deleted. Capacity is in pointers,
// with 0x3FFFFFFF as the byte-overflow limit (four bytes per pointer).
void CaddieEntryList_Append(CaddieEntryVector* self, void* const* position) {
    if (self->size < self->capacity) {
        unsigned int index;
        void* value;
        index = ++self->size;
        value = *position;
        self->first[index - 1] = value;
    } else {
        // Slow path: full, so compute the new capacity (std::vector growth rule above), allocate,
        // move the old pointers across, then swap the new array in and free the old one.
        unsigned int needed;
        if ((unsigned)kVecMaxPointers - self->capacity < 1)
            vector_length_error();
        needed = self->size + 1 - self->capacity;
        unsigned int half;
        unsigned int three_fifths;
        VectorGrow grow(&self->capacity);
        unsigned int cap = *(volatile unsigned int*)&self->capacity;
        if (needed > (unsigned)kVecMaxPointers - cap)
            vector_length_error();
        if (cap < (unsigned)kVecGrowThreshold1) {
            three_fifths = (3 * (cap + 1)) / 5;
            cap += vector_max(three_fifths, needed);
        } else if (cap < (unsigned)kVecGrowThreshold2) {
            half = (cap + 1) / 2;
            cap += vector_max(half, needed);
        } else
            cap = kVecMaxPointers;
        if (cap > (unsigned)kVecMaxPointers)
            vector_length_error();
        void** data = (void**)appMallocDummy__FUl(cap * 4);
        grow.offset = self->size;
        (data + grow.offset)[grow.size++] = *position;
        void** old = self->first;
        void** end = old + self->size;
        grow.first = data;
        grow.capacity = cap;
        int count = end - old;
        grow.offset -= count;
        memcpy(data + grow.offset, old, count * 4);
        memset(old, 0, count * 4);
        grow.size += count;
        self->size = 0;
        unsigned int oldcap = self->capacity;
        self->capacity = grow.capacity;
        grow.capacity = oldcap;
        void** olddata = self->first;
        self->first = grow.first;
        grow.first = olddata;
        self->size = grow.size;
        grow.size = 0;
    }
}
#pragma use_lmw_stmw reset

struct CaddieTreeNodeView {
    void* left;
    void* right;
    char pad08[4];
};
struct Fn8012900CString {
    unsigned int flags;
    unsigned int length;
    const char* data;
};
void CaddieDialogueStringMap_DestroySubtree(void*, void*);
void CaddieDialogueStringPair_Destroy(void*, void*);
void* Table_ResolveDataOffset(void*, unsigned int);
const char* OffsetStringNode_GetText(void*);
void WString_ReplaceRange(void*, unsigned int, unsigned int, const void*, const void*, const void*);
void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const void*);
void String_ReserveCapacity(Fn8012900CString*, unsigned int);
void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
void NString_AssignCString(void*, const char*);
void* Table_GetEntry(void*, int);
void Resource_Release(void*);
void WString_ReserveCapacity(void*, unsigned int);
void* StringWStringMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
extern char caddieDialogueModelPathTable[];
extern void* g_resourceManager;
extern char __vt__10CSerifuEcb[];

// The allocator byte is copied for the library ABI and has no gameplay value.
struct LoadAllocator {
    volatile char byte;
    char pad[3];
    // Preserve the library's opaque allocator byte without gameplay initialization.
    LoadAllocator() {}
    // Copy the opaque allocator token required by string operations.
    LoadAllocator(const LoadAllocator& s) : byte(s.byte) {}
};
struct LoadString {
    unsigned int flags, length;
    char* data;
    // Create an empty narrow small-string owner.
    LoadString() : flags(0), length(0), data(0) {}
    // Return the element count from the inline tag or heap length, excluding NUL.
    unsigned int size() const {
        unsigned int n;
        if (!(flags >> 31))
            n = *(const unsigned char*)this & 0x7f;
        else
            n = length;
        return n;
    }
    // Borrow narrow text from inline storage or the owned heap allocation.
    char* begin() const {
        char* p;
        if (!(flags >> 31))
            p = (char*)this + 1;
        else
            p = data;
        return p;
    }
    // Release heap bytes only; inline bytes belong to the object.
    ~LoadString() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Deep-copy heap text or copy the inline representation verbatim.
    LoadString(const LoadString& s) {
        unsigned int f = s.flags;
        if (!(f >> 31)) {
            flags = f;
            length = s.length;
            data = s.data;
        } else {
            unsigned int n = s.length;
            flags = 0;
            length = 0;
            data = 0;
            String_ReserveCapacity((Fn8012900CString*)this, n);
            LoadAllocator alloc, copy = alloc;
            String_ReplaceRange(this, 0, 0, s.data, s.data + s.length, &copy);
        }
    }
};
struct LoadWide {
    unsigned int flags, length;
    wchar_t* data;
    // Create an empty wide small-string owner.
    LoadWide() : flags(0), length(0), data(0) {}
    // Release only separately allocated wide text.
    ~LoadWide() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return the element count from the inline tag or heap length, excluding NUL.
    unsigned int size() const {
        unsigned int n;
        if (!(flags >> 31))
            n = *(const unsigned char*)this & 0x7f;
        else
            n = length;
        return n;
    }
    // Deep-copy heap wide text or copy the inline representation.
    LoadWide(const LoadWide& s) {
        unsigned int f = s.flags;
        if (!(f >> 31)) {
            flags = f;
            length = s.length;
            data = s.data;
        } else {
            unsigned int n = s.length;
            flags = 0;
            length = 0;
            data = 0;
            WString_ReserveCapacity(this, n);
            LoadAllocator a, c = a;
            WString_ReplaceRange(this, 0, 0, s.data, s.data + s.length, &c);
        }
    }
};
struct LoadValue {
    unsigned char type;
    LoadWide text;
    // Default-construct text; type is deliberately filled by the table loader.
    LoadValue() {}
    // Copy the value type and independently owned wide text.
    LoadValue(const LoadValue& s) : type(s.type), text(s.text) {}
};
// Construct a tree payload by deep-copying both the narrow key and wide value.
struct LoadPair {
    LoadString key;
    LoadValue value;
    LoadPair(const LoadString& k, const LoadValue& v) : key(k), value(v) {}
};
struct LoadFile {
    virtual void slot0();
    virtual void slot1();
    virtual void read(void*, unsigned int);
    unsigned int length;
};
struct LoadFileGuard_u80128808 {
    LoadFile** ptr;
    // Watch a local handle slot until the loader explicitly releases it.
    LoadFileGuard_u80128808(LoadFile** p) : ptr(p) {}
    // Release an outstanding resource handle on early return or exception.
    ~LoadFileGuard_u80128808() {
        if (*ptr) {
            if (*ptr)
                Resource_Release(*ptr);
            *ptr = 0;
        }
    }
};
struct LoadDocument {
    void* data;
    void* volatile table;
    // Borrow the loaded bytes and install the dialogue-table dispatch table.
    LoadDocument(void* p) : data(0) {
        if (p)
            data = p;
        table = __vt__10CSerifuEcb;
    }
    // The loader, not this adapter, owns the table bytes.
    ~LoadDocument() {}
};
struct LoadEntry {
    unsigned int name, type, value;
};
struct LoadIterator {
    LoadDocument* doc;
    int index;
    // Begin before the first dialogue-table row.
    LoadIterator(LoadDocument* d) : doc(d), index(-1) {}
    // Borrow the next row or report exhaustion; the document retains storage.
    bool next(LoadEntry*& entry) {
        int count = doc->data ? ((DialogueTableHeader*)doc->data)->entryCount : 0;
        if (index >= count - 1) {
            entry = 0;
            return false;
        }
        index = index == -1 ? 0 : index + 1;
        entry = (LoadEntry*)Table_GetEntry(doc->data, index);
        return entry != 0;
    }
};
// Borrow the smaller byte count for bounded dialogue-key comparison.
static inline const unsigned int& load_min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Dialogue keys use the same inline/owned string comparison as tree insertion.
static inline int load_compare(const LoadString& a, const LoadString& b) {
    unsigned int n;
    const char* q;
    if (!(b.flags >> 31)) {
        q = (const char*)&b + 1;
        n = *(const unsigned char*)&b & 0x7f;
    } else {
        q = b.data;
        n = b.length;
    }
    unsigned int len;
    if (!(a.flags >> 31))
        len = *(const unsigned char*)&a & 0x7f;
    else
        len = a.length;
    unsigned int other;
    const char* p;
    if (!(a.flags >> 31)) {
        p = (const char*)&a + 1;
        other = *(const unsigned char*)&a & 0x7f;
    } else {
        p = a.data;
        other = a.length;
    }
    unsigned int requested = n;
    unsigned int count = load_min(len, other);
    int cmp = memcmp(p, q, load_min(requested, count));
    if (!cmp) {
        if (count < n)
            return -1;
        if (count == n)
            return 0;
        return 1;
    }
    return cmp;
}

// Destroy child dialogue nodes and their owned narrow/wide strings before freeing this node.
static inline void DestroyDialogueNode(void* owner, CaddieTreeNodeView* n) {
    if (n == 0)
        return;
    CaddieTreeNodeView* left = (CaddieTreeNodeView*)n->left;
    if (left != 0) {
        if (*(void**)left)
            CaddieDialogueStringMap_DestroySubtree(PrimaryCaddieDialogue(owner), *(void**)left);
        if (left->right)
            CaddieDialogueStringMap_DestroySubtree(PrimaryCaddieDialogue(owner), left->right);
        CaddieDialogueStringPair_Destroy(PrimaryCaddieDialogue(owner), ((CaddieTreeNode*)left)->pair);
        appFreeUni__FPv(left);
    }
    CaddieTreeNodeView* right = (CaddieTreeNodeView*)n->right;
    if (right != 0) {
        if (*(void**)right)
            CaddieDialogueStringMap_DestroySubtree(PrimaryCaddieDialogue(owner), *(void**)right);
        if (right->right)
            CaddieDialogueStringMap_DestroySubtree(PrimaryCaddieDialogue(owner), right->right);
        CaddieDialogueStringPair_Destroy(PrimaryCaddieDialogue(owner), ((CaddieTreeNode*)right)->pair);
        appFreeUni__FPv(right);
    }
    CaddieDialogueStringPair_Destroy(PrimaryCaddieDialogue(owner), ((CaddieTreeNode*)n)->pair);
    appFreeUni__FPv(n);
}

#pragma use_lmw_stmw on
struct LoadOwner {
    char pad00[0x10];
    IFF_STRUCT::sCaddie* caddieRecord;
    char pad14[4];
    unsigned int count;
    void* root;
    char pad20[4];
    void** end;
};
#pragma push
#pragma always_inline on
// Destroy the old dialogue subtree and reset the entry header before loading replacement text.
static inline void ClearCaddieDialogueForReload(LoadOwner* dialogueOwner, void* entry, void*& oldRoot,
                                                CaddieTreeNodeView*& subtree) {
    oldRoot = dialogueOwner->root;
    if (oldRoot) {
        subtree = *(CaddieTreeNodeView**)oldRoot;
        if (subtree) {
            DestroyDialogueNode(entry, (CaddieTreeNodeView*)subtree->left);
            DestroyDialogueNode(entry, (CaddieTreeNodeView*)subtree->right);
            CaddieDialogueStringPair_Destroy(PrimaryCaddieDialogue(entry), ((CaddieTreeNode*)subtree)->pair);
            appFreeUni__FPv(subtree);
        }
        subtree = (CaddieTreeNodeView*)((CaddieTreeNodeView*)oldRoot)->right;
        if (subtree) {
            DestroyDialogueNode(entry, (CaddieTreeNodeView*)subtree->left);
            DestroyDialogueNode(entry, (CaddieTreeNodeView*)subtree->right);
            CaddieDialogueStringPair_Destroy(PrimaryCaddieDialogue(entry), ((CaddieTreeNode*)subtree)->pair);
            appFreeUni__FPv(subtree);
        }
        CaddieTreeNodePair_dtor((CaddieTreeNodePairView*)(((CaddieTreeNode*)oldRoot)->pair), -1);
        appFreeUni__FPv(oldRoot);
        dialogueOwner->count = 0;
        dialogueOwner->root = 0;
        dialogueOwner->end = &dialogueOwner->root;
    }
}
#pragma pop

// Replace the entry's dialogue tree by loading its model-specific .eky table of wide text values.
// arg is a real entry with a valid caddieRecord/model. self is unused here.
// Existing dialogue is destroyed before IO; open failure leaves the map empty.
// Eight model/path pairs are searched; a miss still tries the bare extension.
// Values are deep-copied into the entry; parser views borrow the temporary bytes.
// Repeated keys replace their type/text rather than append duplicate nodes.
void CaddieDialogue_LoadTextMap(void* self, void* arg) {
    LoadOwner* owner = (LoadOwner*)arg;
    // Phase 1: postorder teardown, including both strings, then reset the sentinel.
    void* oldRoot;
    CaddieTreeNodeView* subtree;
    ClearCaddieDialogueForReload(owner, arg, oldRoot, subtree);
    // Phase 2: choose the model path and append .eky; preserve the fallback on miss.
    LoadString path;
    struct NamePair {
        const char *name, *path;
    };
    NamePair* names = (NamePair*)caddieDialogueModelPathTable;
    for (unsigned int i = 0; i < 8; ++i, ++names) {
        if (strcmp(names->name, owner->caddieRecord->model) == 0) {
            const char* s = ((NamePair*)caddieDialogueModelPathTable)[i].path;
            unsigned int count;
            count = (!(path.flags >> 31)) ? *(unsigned char*)&path & 0x7f : path.length;
            LoadAllocator alloc, copy = alloc;
            String_ReplaceRange(&path, 0, count, s, s + strlen(s), &copy);
            break;
        }
    }
    {
        const char* s = golfCaddieStrings + kCaddieStringsDialogueExtensionOffset;
        unsigned int count;
        count = (!(path.flags >> 31)) ? *(unsigned char*)&path & 0x7f : path.length;
        LoadAllocator alloc, copy = alloc;
        String_ReplaceRange(&path, count, 0, s, s + strlen(s), &copy);
    }
    const char* path_data;
    path_data = (!(path.flags >> 31)) ? (char*)&path + 1 : path.data;
    NString_AssignCString(&path, path_data);
    path_data = (!(path.flags >> 31)) ? (char*)&path + 1 : path.data;
    // Phase 3: read exactly length bytes plus a local NUL and release the file handle.
    LoadFile* file = (LoadFile*)Resource_OpenResolvedPath(g_resourceManager, path_data, -1, 0, 0, 0);
    LoadFileGuard_u80128808 guard(&file);
    if (!file)
        return;
    unsigned int length = file->length;
    char* buffer = (char*)__nwa__FUl(length + 1);
    file->read(buffer, length);
    buffer[length] = 0;
    if (file) {
        Resource_Release(file);
        file = 0;
    }
    LoadDocument doc(buffer);
    void* data = doc.data;
    LoadIterator iter(&doc);
    LoadEntry* entry;
    // Phase 4: resolve offset-based table names/wide values, find-or-insert by key,
    // and deep-copy text. Temporary key/value owners release their own buffers.
    while (iter.next(entry)) {
        void* predecessor;
        unsigned char insertLeft, noPredecessor;
        void* insertionParent;
        LoadValue value;
        value.type = entry->type;
        const wchar_t* wide = (const wchar_t*)Table_ResolveDataOffset(doc.data, entry->value);
        unsigned int old;
        if (!(value.text.flags >> 31))
            old = *(unsigned char*)&value.text & 0x7f;
        else
            old = value.text.length;
        LoadAllocator walloc, wcopy = walloc;
        WString_ReplaceRange(&value.text, 0, old, wide, wide + wcslen(wide), &wcopy);
        const char* name = OffsetStringNode_GetText(Table_ResolveDataOffset(data, entry->name));
        LoadString key;
        unsigned int len = strlen(name);
        String_ReserveCapacity((Fn8012900CString*)&key, len);
        LoadAllocator alloc, copy = alloc;
        String_ReplaceRange(&key, 0, 0, name, name + len, &copy);
        insertionParent = CaddieDialogueTextMap_FindInsertionPosition(PrimaryCaddieDialogue(arg), (NStringView*)&key,
                                                                      &predecessor, &insertLeft, &noPredecessor);
        LoadPair* destination;
        // Insert when there is no predecessor or its key is strictly smaller.
        if (!predecessor || (destination = (LoadPair*)(((CaddieTreeNode*)predecessor)->pair),
                             load_compare(destination->key, key) < 0)) {
            LoadValue empty;
            LoadPair pair(key, empty);
            destination = (LoadPair*)((char*)StringWStringMap_InsertNode(PrimaryCaddieDialogue(arg), insertionParent,
                                                                         insertLeft, noPredecessor, &pair) +
                                      0xC);
        }
        destination->value.type = value.type;
        LoadWide& dst = destination->value.text;
        unsigned int is_long = dst.flags >> 31;
        if (!is_long && !(value.text.flags >> 31)) {
            dst = value.text;
        } else {
            unsigned int n;
            if (is_long)
                n = dst.length;
            else
                n = *(unsigned char*)&dst & 0x7f;
            const wchar_t* p;
            unsigned int size;
            if (!(value.text.flags >> 31)) {
                p = (wchar_t*)((char*)&value.text + 2);
                size = *(unsigned char*)&value.text & 0x7f;
            } else {
                p = value.text.data;
                size = value.text.length;
            }
            LoadAllocator a, c = a;
            WString_ReplaceRange(&dst, 0, n, p, p + size, &c);
        }
    }
    // Phase 5: tree strings now own independent storage; discard the parse buffer.
    __dla__FPv(buffer);
}

} // extern "C"

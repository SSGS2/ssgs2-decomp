// Party and versus mode main tasks: CPartyMain and CVSMain with their BP entries, loading and
// cleanup, and the render-context save data that holds each player's character record.

#include <decomp/static_init.h>
#include "MSL/error_message.h"
#include "classes/CPartyMain.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/MWStringStorage.h"
#include "game/vector.h"

#include "game/singletons.h"
#include "classes/CVSMain.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CBpCtrl.h"

#define AsCVSMain(p) ((CVSMain*)(p))
extern "C" {

extern float kFloat_6_2831855;
extern float s_renderContextTwoPi;
extern float s_versusMainTwoPi;
extern void* g_soundManager;
extern signed char s_bpCtrlInitialized;
extern unsigned char s_partyItemDetailDrawFirst;
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char __dt__Q23std12length_errorFv[];
extern char renderNameTreeExceptionStrings[];
extern char s_sharedCharDeleterVtable[];

void RenderNameTree_DestroyValue(void* a0, unsigned int* a1);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
void CharacterLoading_CreateAnimationEntry();
void CharacterLoading_DeleteAnimationEntry();
void* RenderContext_GetOrCreate();
void SoundManager_RequestBgmStop(void*, int);
void __ct__18NativeBase80201D18Fv(void*);
void CBpCtrl_dtor();
void __register_global_object(void*, void (*)(), void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void CourseSelection_ReleaseAndFree(void*, int);
void destroy__9CTaskMainFi(void*, int);
void __dl__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void WString_ReserveCapacity(void*, unsigned int);
void WString_ReplaceRange(void*, int, int, void*, void*, void*);
void RBTree_RebalanceAfterInsert(void*, void*, unsigned int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
unsigned int strlen(const char* s);
char* strcpy(char* dst, const char* src);
#pragma does_not_return __throw
void __throw(const char* typeinfo, void* object, void* dtor);
void __dla__FPv(void*);

enum PartyConstants {
    kPlayerSlotCount = 4,
    kPlayerRecordBytes = sizeof(SavePlayerRecord),
    kSlotVectorBytes = 12,
    kSlotVectorsOffset = kPlayerSlotCount * kPlayerRecordBytes,
    kEnemyRenderTableOffset = kSlotVectorsOffset + kPlayerSlotCount * 12,
    kEnemyRenderRecordBytes = 0xA4,
    kInitialIndexCapacity = 4,
    kInvalidEntryId = -1,
    kTaskMainVtableOffset = 0x3C,
    kInitialPartyItem = 9,
    kLengthErrorTypeOffset = 0x1A,
    kPartySelectionBgmNameOffset = 0x26,
    kSecondaryBpStringOffset = 0x11
};
// Context layout: four 0x828-byte saved-player records at +0, four 12-byte
// index vectors at +0x20A0, render table at +0x20D0. Each vector owns its int
// buffer and uses element counts, not byte counts. Its last element is the
// selected render-record index; empty/invalid slots produce -1.
// GetCurrentSlotRenderRecord forwards -1 to the render-table resolver; it does
// not itself guarantee a null/invalid result. All returned records are borrowed.
struct RenderSlotIndices {
    int* values;
    int count;
    unsigned int capacity;
};
struct PartyRenderContextView {
    SavePlayerRecord players[kPlayerSlotCount];
    RenderSlotIndices slotIndices[kPlayerSlotCount];
    CEnemyRenderTable renderTable;
};

struct RenderIndexArrayPrefix {
    int* values;
    int count;
};

// Only the prefix/slot-index region is needed by these helpers.
struct RenderContextPrefix {
    char savedPlayers[kSlotVectorsOffset];
    RenderSlotIndices slotIndices[kPlayerSlotCount];
};

struct RenderNameRange {
    void* begin;
    unsigned int stringFlags;
    unsigned int stringLength;
    void* data;
};

// Render-name tree nodes have three link words followed by a 16-byte value:
// opaque associated pointer at +0xC, tagged UTF-16 string at +0x10.
// The pointer is borrowed; only a heap-backed string (flag bit 31) is owned.
// Tree.root's address is the sentinel/header storage. Insertion assumes a valid
// parent/side from the caller's search; it does not check ordering/duplicates.
// The value starts after the tree's left, right and parent links.
struct RenderNameNode {
    void* left;
    void* right;
    void* parent;
    RenderNameRange value;
};
struct RenderNameTree {
    unsigned int count;
    void* root;
    void* allocator;
    void* firstNode; // Cached first node; updated only when insertion becomes first.
};
struct RenderNameAllocation {
    void* allocator;
    void* pointer;
};
typedef char RenderNameNode_size[(sizeof(RenderNameNode) == 0x1C) ? 1 : -1];
typedef char RenderContext_offset[((int)&((PartyRenderContextView*)0)->renderTable == 0x20D0) ? 1 : -1];

struct RenderNameException {
    void* vtable;
    char* text;
    void* rep;
};

struct RenderNameStringRep {
    void* vtable;
    unsigned int refs;
    unsigned int controlWord08;
    char* data;
};

struct RenderNameCleanupFrame {
    char sso[8];
    unsigned char flag18;
    char pad2[3];
    void* alloc1C;
    void* alloc20;
    char pad3[4];
    void* owner28;
    void* ptr2C;
    void* selfNext30;
    void* pending34;
    RenderNameException exc;
};

// Placement-delete unwind hook: storage remains with the allocation guard.
inline void operator delete(void*, void*) {}

struct U80283608Alloc {
    char c __attribute__((aligned(4)));
};
struct RenderNameValue {
    void* first;
    unsigned int stringFlags;
    unsigned int stringLength;
    void* data;
    // Copy the borrowed associated pointer and deep-copy only heap-backed UTF-16 text.
    RenderNameValue(const RenderNameRange* r) {
        char allocA;
        volatile U80283608Alloc allocB;
        void* f;
        unsigned int n;
        n = r->stringFlags;
        f = r->begin;
        first = f;
        if ((n >> 31) == 0) {
            stringFlags = n;
            unsigned int c = r->stringLength;
            void* d = r->data;
            stringLength = c;
            data = d;
        } else {
            stringFlags = 0;
            unsigned int reserve = r->stringLength;
            stringLength = 0;
            data = 0;
            WString_ReserveCapacity(&stringFlags, reserve);
            int a = allocB.c;
            unsigned int c = *(volatile unsigned int*)&r->stringLength;
            allocA = a;
            void* d = *(void* volatile*)&r->data;
            WString_ReplaceRange(&stringFlags, 0, 0, d, (char*)d + c * 2, (void*)&allocA);
        }
    }
};

struct RenderNameParentLink {
    void* p;
    // Initialize the new node's parent link in place.
    RenderNameParentLink(void* x) : p(x) {}
};

// Construct the parent-link word without changing the tree's link layout.
inline void SetNodeParent(void* node, void* position) {
    new (&((RenderNameNode*)node)->parent) RenderNameParentLink(position);
}

struct RenderNameTemporaries {
    char c;
    void* volatile p;
    void* volatile q;
};
struct ExceptionTextGuard {
    char* text;
    char* allocator;
    // Guard newly allocated exception text until its control block owns it.
    ExceptionTextGuard(char* t, char* a) : text(t), allocator(a) {}
    // Release exception text unless ownership has been transferred.
    ~ExceptionTextGuard() {
        if (text)
            __dla__FPv(text);
    }
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control U80283608Control;
struct RenderNameError {
    void* table;
    char* text;
    void* control;
};
struct NodeAllocationHolder {
    void* alloc;
    void* ptr;
    // Free an uncommitted tree-node allocation during exception unwind.
    ~NodeAllocationHolder() {
        if (ptr)
            appFreeUni__FPv((void*)((unsigned int)ptr));
    }
};

// Throw the standard length error while preserving ownership of its copied message.
inline void U80283608ThrowLength() {
    RenderNameError* e;
    const char* message = renderNameTreeExceptionStrings;
    RenderNameError error;
    U80283608Alloc allocator;
    volatile U80283608Alloc first;
    error.table = __vt__Q23std11logic_error;
    e = &error;
    first.c = 0;
    allocator.c = 0;
    unsigned int len = strlen(message) + 1;
    error.text = (char*)__nwa__FUl(2 * len - len);
    {
        char* p = error.text;
        ExceptionTextGuard guard(p, &allocator.c);
        error.control = new U80283608Control(p);
        guard.text = 0;
    }
    strcpy(error.text, message);
    e->table = __vt__Q23std12length_error;
    __throw(renderNameTreeExceptionStrings + kLengthErrorTypeOffset, e, (void*)__dt__Q23std12length_errorFv);
}

#pragma push
#pragma use_lmw_stmw on
// Allocate and copy a render-name tree value, link the node, rebalance, and update the cached first node when requested.
void* RenderNameTree_InsertNode(void* self, void* position, int before, int becomesFirst, RenderNameRange* range) {
    if (((RenderNameTree*)self)->count == 0xFFFFFFFF)
        U80283608ThrowLength();
    volatile U80283608Alloc allocator;
    void* volatile q;
    void* volatile p;
    allocator.c = 0;
    // Allocate/construct under a guard; no tree link owns the new node yet.
    void* allocation = appMallocDummy__FUl(sizeof(RenderNameNode));
    p = allocation;
    q = allocation;
    void* values = &((RenderNameNode*)allocation)->value;
    NodeAllocationHolder holder = {&((RenderNameTree*)self)->root, allocation};
    new (values) RenderNameValue(range);
    // Commit the node links, then update count and rebalance. String construction has succeeded.
    void* node = holder.ptr;
    *(unsigned int*)&((RenderNameNode*)node)->right = 0;
    holder.ptr = 0;
    *(unsigned int*)&((RenderNameNode*)node)->left = 0;
    SetNodeParent(node, position);
    if (before)
        ((RenderNameNode*)position)->left = node;
    else
        ((RenderNameNode*)position)->right = node;
    unsigned int count = ((RenderNameTree*)self)->count;
    ((RenderNameTree*)self)->count = count + 1;
    RBTree_RebalanceAfterInsert(node, ((RenderNameTree*)self)->root, count);
    if (becomesFirst)
        ((RenderNameTree*)self)->firstNode = node;
    return node;
}
#pragma pop

// Release a temporary render-name allocation and optionally delete its holder.
void* RenderNameAllocation_dtor(void* obj, int flag) {
    if (obj) {
        void* ptr = ((RenderNameAllocation*)obj)->pointer;
        if (ptr)
            appFreeUni__FPv((void*)((unsigned int)ptr));
        if (flag > 0)
            __dl__FPv(obj);
    }
    return obj;
}

// Return the last render-record index for a player slot, or -1 for an invalid or empty slot.
int RenderContext_GetLastSlotRecordIndex(RenderContextPrefix* obj, int index) {
    RenderIndexArrayPrefix* array;
    if (index < 0 || index >= kPlayerSlotCount)
        array = 0;
    else {
        obj = (RenderContextPrefix*)((char*)obj + index * kSlotVectorBytes);
        array = (RenderIndexArrayPrefix*)(((PartyRenderContextView*)obj)->slotIndices);
    }
    if (!array)
        return -1;
    if (array->count == 0)
        return -1;
    return array->values[*(volatile int*)&array->count - 1];
}

// Return a vector's last index, treating null and empty vectors as -1.
static inline int RenderIndexArray_Back(RenderIndexArrayPrefix* array) {
    if (!array)
        return -1;
    if (!*(volatile int*)&array->count)
        return -1;
    return array->values[array->count - 1];
}

// Resolve a slot's last selected index through the enemy-render table.
int RenderContext_GetCurrentSlotRenderRecord(RenderContextPrefix* obj, int index) {
    RenderIndexArrayPrefix* array;
    int value;
    if (index < 0 || index >= kPlayerSlotCount)
        array = 0;
    else {
        index = (int)obj + index * kSlotVectorBytes;
        array = (RenderIndexArrayPrefix*)(index + kSlotVectorsOffset);
    }
    if (array == 0) {
        value = -1;
    } else {
        if (*(volatile int*)&array->count == 0) {
            value = -1;
        } else {
            value = array->values[*(volatile int*)&array->count - 1];
        }
    }
    return (int)CCharacterPreviewTable::GetRecordByIndex(&((PartyRenderContextView*)obj)->renderTable, value);
}

// Copy-granularity view of SavePlayerRecord: retain word/halfword groups for CodeWarrior.
// The first block includes character/equipment at +0x2C and unlockedCourses at +0x258;
// remaining block roles are unresolved. Use SavePlayerRecord for semantic field access.
struct SavedPlayerCopy {
    int throughUnlockedCourses[151];
    int words25C[28];
    short halfwords2CC[208];
    int words46C[16];
    char bytes4AC[24];
    char bytes4C4[6];
    char bytes4CA[8];
    char bytes4D2[3];
    int words4D8[5];
    short halfwords4EC[414];
};
typedef char SavedPlayerCopy_size[(sizeof(SavedPlayerCopy) == sizeof(SavePlayerRecord)) ? 1 : -1];

struct RenderIndexVector {
    void* begin;
    int size;
    unsigned int capacity;
};

void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);

struct RenderIndexIterator {
    int* p;
    // Borrow the current position in a contiguous render-index array.
    RenderIndexIterator(int* q) : p(q) {}
};

// Copy index words to dest, zero the source words and return the number of elements moved.
inline int MoveRenderIndices(RenderIndexIterator first, RenderIndexIterator last, int* dest) {
    int n = last.p - first.p;
    memcpy(dest, first.p, n * 4);
    memset(first.p, 0, n * 4);
    return n;
}

// Temporary replacement array for the render-index vector reserve (keeps its address-based name: the destructor symbol embeds it).
struct U80283TmpVec {
    int* begin;
    int size;
    unsigned int capacity;
    // Start an empty temporary that owns any subsequently assigned buffer.
    U80283TmpVec() : begin(0), size(0), capacity(0) {}
    // Free the allocation left behind by a reserve swap.
    ~U80283TmpVec() {
        if (begin) {
            size -= (unsigned int)size;
            appFreeUni__FPv((void*)((unsigned int)begin));
        }
    }
};

struct RenderIndexVectorMethods {
    int* begin;
    int size;
    unsigned int capacity;
    // Clear the logical vector without releasing its capacity.
    void clear() {
        size -= (unsigned int)size;
    }
    // Ensure room for four indices; swap old storage into the temporary for cleanup.
    void reserve() {
        if (capacity < kInitialIndexCapacity) {
            U80283TmpVec t;
            int sz = t.size;
            int* p2 = (int*)appMallocDummy__FUl(0x10);
            t.begin = p2;
            t.capacity = kInitialIndexCapacity;
            int* oldBegin = begin;
            int n = (begin + size) - oldBegin;
            memcpy(p2 + sz, oldBegin, n * 4);
            memset(oldBegin, 0, n * 4);
            size = 0;
            t.size += n;
            int* tb = begin;
            begin = t.begin;
            t.begin = tb;
            int ts = size;
            size = t.size;
            t.size = ts;
            unsigned int tc = capacity;
            capacity = t.capacity;
            t.capacity = tc;
        }
    }
};

struct RenderContextCopyView {
    SavedPlayerCopy items[4];
    RenderIndexVectorMethods vecs[4];
};

#pragma push
#pragma use_lmw_stmw on
#pragma push
#pragma always_inline on
// Publish the shared save-data storage after its one-time construction.
static inline void EnsureRenderContextSaveData() {
    extern signed char s_saveDataInitialized;
    extern char s_saveData[];
    extern void SaveData_Ctor(void*);
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
}
#pragma pop

// Initialize four saved-player copies, slot-index vectors and render records, then return the singleton.
void* RenderContext_GetOrCreate() {
    extern signed char s_renderContextInitialized;
    extern signed char s_saveDataInitialized;
    extern char s_renderContext[];
    extern char s_saveData[];
    extern char s_renderContextDtorRegistration_80283608[];
    extern void SaveData_InitPlayerRecord(void*);
    extern void __construct_array(void*, void (*)(void*), void (*)(void*, int), int, int);
    extern void SaveData_Ctor(void*);
    extern void* SaveData_GetCurrentRecord(void*);
    extern void __dt__13RenderContextFv();
    extern void* memcpy(void*, const void*, unsigned int);
    extern void* memset(void*, int, unsigned int);
    extern void PartyItemRecord_InitDefaults(void*);

    extern void PlayerRenderRecord_dtor(void*, int);

    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            // Phase 1: initialize saved records, empty vectors and default render records.
            char* base = s_renderContext;
            char* p = base;
            char* pos = base + kSlotVectorsOffset;
            do {
                SaveData_InitPlayerRecord(p);
                p += kPlayerRecordBytes;
            } while (p < pos);
            ((IntVectorArray*)(pos))->__ct();
            pos = base + kEnemyRenderTableOffset;
            __construct_array(pos, PartyItemRecord_InitDefaults, PlayerRenderRecord_dtor, kEnemyRenderRecordBytes,
                              kPlayerSlotCount);
            ((CEnemyRenderTable*)pos)->count = 0;
            // Phase 2: reserve each selection vector and seed every player from the active save.
            RenderContextCopyView* g = (RenderContextCopyView*)base;
            int i;
            for (i = 0; i < kPlayerSlotCount; i++) {
                RenderIndexVectorMethods* v =
                    (RenderIndexVectorMethods*)(base + i * kSlotVectorBytes + kSlotVectorsOffset);
                v->clear();
                if (v->capacity < kInitialIndexCapacity) {
                    U80283TmpVec t;
                    int sz = t.size;
                    int* p2 = (int*)appMallocDummy__FUl(0x10);
                    RenderIndexVectorMethods* w =
                        (RenderIndexVectorMethods*)((char*)g + i * kSlotVectorBytes + kSlotVectorsOffset);
                    t.begin = p2;
                    t.capacity = kInitialIndexCapacity;
                    int n = MoveRenderIndices(w->begin, (int*)((char*)w->begin + w->size * 4), t.begin + t.size);
                    w->size = 0;
                    t.size += n;
                    unsigned int tc = w->capacity;
                    w->capacity = t.capacity;
                    t.capacity = tc;
                    int* tb = w->begin;
                    w->begin = t.begin;
                    t.begin = tb;
                    int ts = w->size;
                    w->size = t.size;
                    t.size = ts;
                }
                EnsureRenderContextSaveData();
                *(SavedPlayerCopy*)(base + i * kPlayerRecordBytes) =
                    *(SavedPlayerCopy*)SaveData_GetCurrentRecord(GetSaveDataSlot());
            }
            // Publish only after initialization; destructor registration owns final cleanup.
            __register_global_object(s_renderContext, __dt__13RenderContextFv,
                                     s_renderContextDtorRegistration_80283608);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    return Sing_805DC648();
}
#pragma pop

// Default party-task hook: perform no work.
void PartyTask_OnInitNoop() {}

// Default party-task hook: perform no work.
void CVSMain::OnPreLoadInit() {}

// Initialize this versus-task unit's cached two-pi constant.
void CVSMain_InitializeTwoPi() {
    s_versusMainTwoPi = kFloat_6_2831855;
}

// Request the party-selection BGM and create the shared character animation entry.
void CPartyMain::OnLoad() {
    extern const char titleTaskStrings[];
    SoundManager_RequestBgmChange(g_soundManager, titleTaskStrings + kPartySelectionBgmNameOffset, 0, 1);
    CharacterLoading_CreateAnimationEntry();
}

struct VersusMainStorage {
    void* vt;
    char padding04[0x14];
    void* vt2;
    int processState;
    int ownedController;
    int primaryBpEntryId;
    int secondaryBpEntryId;
    int menuState;
    unsigned char selectionFlags;
    unsigned char animationFlags;
    unsigned char inputFlags;
    unsigned char selectedItem;
    int primaryItems;
    int primaryItemCount;
    int primaryItemCapacity;
    int secondaryItems;
    int secondaryItemCount;
    int secondaryItemCapacity;
};
extern char __vt__7CVSMain[];

extern char s_bpCtrl[];
extern char s_partyMainBpCtrlDestructorRecord[];
// Lazy singleton set-up the original pasted inline before every use: if the global BpCtrl pointer
// (Sing_805DC5F0) is still null, construct the static BpCtrl once (guard flag + destructor
// registration) and publish it. Afterwards Sing_805DC5F0() is the BpCtrl.
static inline void EnsurePartyBpCtrl() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_partyMainBpCtrlDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
}

// Construct the versus task, reset selections and item vectors, and create its two BP entries.
CVSMain::CVSMain() {
    VersusMainStorage* obj = (VersusMainStorage*)this;
    __ct__18NativeBase80201D18Fv(obj);
    extern const char titleTaskStrings[];
    extern char s_bpCtrl[];
    extern char s_partyMainBpCtrlDestructorRecord[];
    obj->vt = __vt__7CVSMain;
    obj->vt2 = __vt__7CVSMain + kTaskMainVtableOffset;
    obj->processState = 0;
    obj->ownedController = 0;
    obj->primaryBpEntryId = -1;
    obj->secondaryBpEntryId = -1;
    obj->menuState = 0;
    obj->primaryItems = 0;
    obj->primaryItemCount = 0;
    obj->primaryItemCapacity = 0;
    obj->secondaryItems = 0;
    obj->secondaryItemCount = 0;
    obj->secondaryItemCapacity = 0;
    obj->selectionFlags = 0;
    obj->animationFlags = 0;
    obj->inputFlags = 0;
    obj->selectedItem = kInitialPartyItem;
    s_partyItemDetailDrawFirst = 0;
    EnsurePartyBpCtrl();
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(titleTaskStrings, &obj->primaryBpEntryId, 0);
    EnsurePartyBpCtrl();
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(titleTaskStrings + kSecondaryBpStringOffset, &obj->secondaryBpEntryId, 0);
}

#pragma opt_propagation off
// The stacked `if ((volatile char*)obj + offset)` guards below are inlined vector destructors (always true; the
// address arithmetic is kept so the compiler emits the same code). In plain C each block is:
//   if (items) { itemCount = 0; appFreeUni(items); }
#pragma push
#pragma always_inline on
// Release the owned secondary item buffer, preserving the inlined destructor guards.
static inline void ReleaseVersusSecondaryItems(void* obj) {
    if ((volatile char*)obj + (int)&((CVSMain*)0)->secondaryItems) {
        if ((volatile char*)obj + (int)&((CVSMain*)0)->secondaryItems) {
            if ((volatile char*)obj + (int)&((CVSMain*)0)->secondaryItems) {
                if ((volatile char*)obj + (int)&((CVSMain*)0)->secondaryItems) {
                    if ((volatile char*)obj + (int)&((CVSMain*)0)->secondaryItems) {
                        void* p = AsCVSMain(obj)->secondaryItems;
                        if (p) {
                            int count = *(volatile int*)&AsCVSMain(obj)->secondaryItemCount;
                            int same = count;
                            AsCVSMain(obj)->secondaryItemCount = count - same;
                            appFreeUni__FPv((void*)((unsigned int)p));
                        }
                    }
                }
            }
        }
    }
}
#pragma pop

#pragma push
#pragma always_inline on
// Release the owned primary item buffer, preserving the inlined destructor guards.
static inline void ReleaseVersusPrimaryItems(void* obj) {
    if ((volatile char*)obj + (int)&((CVSMain*)0)->primaryItems) {
        if ((volatile char*)obj + (int)&((CVSMain*)0)->primaryItems) {
            if ((volatile char*)obj + (int)&((CVSMain*)0)->primaryItems) {
                if ((volatile char*)obj + (int)&((CVSMain*)0)->primaryItems) {
                    if ((volatile char*)obj + (int)&((CVSMain*)0)->primaryItems) {
                        void* p = AsCVSMain(obj)->primaryItems;
                        if (p) {
                            int count = *(volatile int*)&AsCVSMain(obj)->primaryItemCount;
                            int same = count;
                            AsCVSMain(obj)->primaryItemCount = count - same;
                            appFreeUni__FPv((void*)((unsigned int)p));
                        }
                    }
                }
            }
        }
    }
}
#pragma pop

// Stop versus music, release both item vectors and destroy the native base.
CVSMain::~CVSMain() {
    extern char __vt__7CVSMain[];
    *(void**)this = __vt__7CVSMain;
    taskMainVtable = __vt__7CVSMain + kTaskMainVtableOffset;
    SoundManager_RequestBgmStop(g_soundManager, 1);
    if ((volatile char*)this + (int)&((VersusMainStorage*)0)->selectionFlags) {
        ReleaseVersusSecondaryItems(this);
        ReleaseVersusPrimaryItems(this);
    }
    if (this) {
        __dt__18NativeBase80201D18Fv(this, 0);
    }
}
#pragma opt_propagation reset

struct U80283608VCall {
    virtual void v0(int);
};

// Remove party animation/render objects, destroy the controller and delete both BP entries.
void CPartyMain::OnDestroy() {
    extern char s_bpCtrl[];
    extern char s_partyMainBpCtrlDestructorRecord[];
    CharacterLoading_DeleteAnimationEntry();
    CCharacterPreviewTable::DestroyRenderObjects(&((PartyRenderContextView*)RenderContext_GetOrCreate())->renderTable);
    struct PartyControllerOwner {
        char prefix[0x20];
        void* controller;
    };
    PartyControllerOwner* o = (PartyControllerOwner*)this;
    if (o->controller) {
        if (o->controller) {
            void* p = o->controller;
            ((U80283608VCall*)p)->v0(1);
        }
        o->controller = 0;
    }
    int* id = (int*)(((char*)&primaryBpEntryId));
    if (*id != -1) {
        EnsurePartyBpCtrl();
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(*id);
        *id = -1;
    }
    id = (int*)(((char*)&secondaryBpEntryId));
    if (*id != -1) {
        EnsurePartyBpCtrl();
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(*id);
        *id = -1;
    }
}

// Initialize this render-context unit's cached two-pi constant.
void RenderContext_InitializeTwoPi() {
    s_renderContextTwoPi = kFloat_6_2831855;
}

// Copy the active save record into the requested player-array element.
void SaveData_CopyCurrentRecordToArray(SavedPlayerCopy* items, int index) {
    extern signed char s_saveDataInitialized;
    extern char s_saveData[];
    extern void SaveData_Ctor(void*);
    extern void* SaveData_GetCurrentRecord(void*);
    EnsureRenderContextSaveData();
    items[index] = *(SavedPlayerCopy*)SaveData_GetCurrentRecord(GetSaveDataSlot());
}
#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Clear each slot-index vector, ensure its initial capacity and reset all four player copies.
void RenderContext_ResetPlayerRecords(RenderContextPrefix* obj) {
    extern signed char s_saveDataInitialized;
    extern char s_saveData[];
    extern void SaveData_Ctor(void*);
    extern void* SaveData_GetCurrentRecord(void*);
    extern void* appMallocDummy__FUl(unsigned int);
    int n;
    RenderContextPrefix* curObj = obj;
    RenderIndexVector* curVec = (RenderIndexVector*)(((PartyRenderContextView*)obj)->slotIndices);
    SavedPlayerCopy* curItem = (SavedPlayerCopy*)obj;
    int* b;
    int i;
    // For each slot: clear count, ensure capacity, copy current save, then advance both views.
    for (i = 0; i < kPlayerSlotCount; i++) {
        int count = *(volatile int*)&((PartyRenderContextView*)curObj)->slotIndices[0].count;
        int same = count;
        ((RenderIndexVector*)(((PartyRenderContextView*)curObj)->slotIndices))->size = count - same;
        if (((RenderIndexVector*)(((PartyRenderContextView*)curObj)->slotIndices))->capacity < kInitialIndexCapacity) {
            U80283TmpVec t;
            int* p2 = (int*)appMallocDummy__FUl(0x10);
            RenderIndexVector* w = (RenderIndexVector*)(((PartyRenderContextView*)curObj)->slotIndices);
            b = (int*)w->begin;
            int* e = b + w->size;
            int sz = t.size;
            n = e - b;
            t.begin = p2;
            t.capacity = kInitialIndexCapacity;
            memcpy(p2 + sz, b, n * 4);
            memset(b, 0, n * 4);
            w->size = 0;
            t.size += n;
            void* ob;
            unsigned int oc = curVec->capacity;
            curVec->capacity = t.capacity;
            ob = w->begin;
            t.capacity = oc;
            w->begin = t.begin;
            t.begin = (int*)ob;
            int os = w->size;
            w->size = t.size;
            t.size = os;
        }
        EnsureRenderContextSaveData();
        *curItem = *(SavedPlayerCopy*)SaveData_GetCurrentRecord(GetSaveDataSlot());
        curObj = (RenderContextPrefix*)((char*)curObj + kSlotVectorBytes);
        curVec = (RenderIndexVector*)((char*)curVec + kSlotVectorBytes);
        curItem = (SavedPlayerCopy*)((char*)curItem + kPlayerRecordBytes);
    }
}
#pragma opt_propagation reset
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
void RenderNameTree_FreeSubtree(void*, void*);
#pragma push
#pragma always_inline on
// Destroy descendants through the owning tree, then release this frontier value and node.
static inline void DestroyRenderNameFrontier(void* tree, RenderNameNode* node) {
    if (node->left)
        RenderNameTree_FreeSubtree(tree, node->left);
    if (node->right)
        RenderNameTree_FreeSubtree(tree, node->right);
    RenderNameTree_DestroyValue(tree, (unsigned int*)&node->value);
    appFreeUni__FPv((void*)((unsigned int)node));
}
#pragma pop

#pragma push
#pragma always_inline on
// Free the heap-backed UTF-16 name when owned, then release its containing tree node.
static inline void ReleaseRenderNameNode(RenderNameNode* node) {
    if (&node->value) {
        if (&node->value.stringFlags) {
            if (node->value.stringFlags >> 31)
                appFreeUni__FPv((void*)((unsigned int)node->value.data));
        }
    }
    appFreeUni__FPv((void*)((unsigned int)node));
}
#pragma pop

// Requires a nonnull real subtree node; frees nodes and string storage, not associated pointers.
// Destroy render-name strings and tree nodes in postorder, preserving the expanded cleanup sequence.
void RenderNameTree_FreeSubtree(void* self, void* node) {
    void* leftRoot;
    void* leftBranch;
    void* leftLeaf;
    // Left subtree: expanded postorder visits preserve the original cleanup/call sequence.
    leftRoot = ((RenderNameNode*)node)->left;
    if (leftRoot) {
        leftBranch = ((RenderNameNode*)leftRoot)->left;
        if (leftBranch) {
            leftLeaf = ((RenderNameNode*)leftBranch)->left;
            if (leftLeaf) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)leftLeaf);
            }
            leftLeaf = ((RenderNameNode*)leftBranch)->right;
            if (leftLeaf) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)leftLeaf);
            }
            ReleaseRenderNameNode((RenderNameNode*)leftBranch);
        }
        leftLeaf = ((RenderNameNode*)leftRoot)->right;
        if (leftLeaf) {
            leftBranch = ((RenderNameNode*)leftLeaf)->left;
            if (leftBranch) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)leftBranch);
            }
            leftBranch = ((RenderNameNode*)leftLeaf)->right;
            if (leftBranch) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)leftBranch);
            }
            ReleaseRenderNameNode((RenderNameNode*)leftLeaf);
        }
        ReleaseRenderNameNode((RenderNameNode*)leftRoot);
    }
    // Right subtree: apply the same postorder string destruction and node release.
    void* rightRoot = ((RenderNameNode*)node)->right;
    void* rightBranch;
    void* rightLeaf;
    if (rightRoot) {
        rightBranch = ((RenderNameNode*)rightRoot)->left;
        if (rightBranch) {
            rightLeaf = ((RenderNameNode*)rightBranch)->left;
            if (rightLeaf) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)rightLeaf);
            }
            rightLeaf = ((RenderNameNode*)rightBranch)->right;
            if (rightLeaf) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)rightLeaf);
            }
            ReleaseRenderNameNode((RenderNameNode*)rightBranch);
        }
        rightBranch = ((RenderNameNode*)rightRoot)->right;
        if (rightBranch) {
            rightLeaf = ((RenderNameNode*)rightBranch)->left;
            if (rightLeaf) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)rightLeaf);
            }
            rightLeaf = ((RenderNameNode*)rightBranch)->right;
            if (rightLeaf) {
                DestroyRenderNameFrontier(self, (RenderNameNode*)rightLeaf);
            }
            ReleaseRenderNameNode((RenderNameNode*)rightBranch);
        }
        ReleaseRenderNameNode((RenderNameNode*)rightRoot);
    }
    // Both children are gone; destroy this root's string and finally its node.
    ReleaseRenderNameNode((RenderNameNode*)node);
}
#pragma pop

// Free heap-backed string storage in one render-name tree value.
void RenderNameTree_DestroyValue(void* tree, unsigned int* valueWords) {
    if (valueWords) {
        if ((unsigned char*)valueWords + 4 == 0)
            return;
        if (valueWords[1] >> 31)
            appFreeUni__FPv((void*)(valueWords[3]));
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623F0", _ctors_u80283608_0, RenderContext_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804623F0", _ctors_u80283608_1, CVSMain_InitializeTwoPi);
#pragma pop

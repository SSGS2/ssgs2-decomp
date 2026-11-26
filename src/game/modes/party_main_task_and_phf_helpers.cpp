// Party mode helpers: the party main and party task objects, the character-count menu they
// create, the course-slot table, and readers for PHF files (signature check and directory lookup).
// The slot table holds five fixed course slots plus an extra-slot range.

#include <decomp/static_init.h>
#include <decomp/cw_string_guard.h>
#include "classes/CSharedDoc.h"
#include "classes/CEnemyRenderTable.h"
#include "game/vector.h"

#include "game/singletons.h"
#include "classes/CPartyMain.h"
#include "classes/CVSMain.h"
#include "classes/CPartyTask.h"
#include "classes/CTask.h"
#include "classes/CCharaNum.h"
// File header and directory entry views retain PHF's original byte layout.
struct PhfHeader {
    char signature[8];
    int entryCount;
    unsigned int dataOffset;
};
struct PhfEntry {
    char name[24];
    unsigned int dataOffset;
    unsigned int dataSize;
};
struct CourseSlotEntry {
    unsigned char data[0x50];
};
struct CourseSlotTable {
    unsigned short selectedIndex;
    unsigned short padding02;
    void* storage;
    CourseSlotEntry* entries;
};
struct PartyMainMessage {
    unsigned int sender;
    int type;
    int partyMode;
};
enum PartyConstants {
    kPartyGameMode = 7,
    kStartPartyMessage = 4,
    kCourseSlotCount = 5,
    kExtraCourseSlotBase = 3,
    kDefaultActorSkipMask = 0x3F
};
// Layout of the shared render context (s_renderContext): an array of 4 player records of
// kRenderPlayerRecordSize bytes, then an int-vector member, then an array of 4 party item records.
enum RenderContextLayout {
    kRenderPlayerRecordSize = 0x828,
    kRenderPlayerRecordsEnd = 0x20A0, // also the offset of the int-vector member
    kRenderItemRecordsOffset = 0x20D0,
    kRenderItemRecordSize = 0xA4,
    kRenderItemRecordCount = 4,
    kCharaNumMenuSize = 0x3C // sizeof(CCharaNum)
};
// Offsets into partyItemSelectStrings of the std:: exception messages and their type names used by
// WString_ReplaceRange's throw blocks.
enum PartyStringOffset {
    kStringRangeError = 0x106,
    kStringRangeErrorType = 0x121,
    kPartyMainClassName = 1,
    kPartyMainActorName = 12,
    kStringLengthError = 0x158,
    kStringLengthErrorType = 0x173
};

extern "C" {

extern void* g_sharedDoc;
extern signed char s_renderContextInitialized;
extern void SaveData_InitPlayerRecord(void*);
extern void __construct_array(void*, void*, void*, int, int);
extern void RenderContext_ResetPlayerRecords(void*);
extern void __register_global_object(void*, void*, void*);
extern void* __nw__FUl(unsigned int);
extern void strcpy(char*, const char*);
extern void* memmove(void*, const void*, unsigned int);
extern unsigned short kWideStringInPlaceTerminator;

extern void PartyItemRecord_InitDefaults(void*);
extern void PlayerRenderRecord_dtor(void*);
extern void __dt__13RenderContextFv(void*);
extern void CharacterPreviewTable_dtor(void*);
extern char s_renderContext[];
extern char
    s_renderContextDtorRegistration_8008B524[]; // Destructor registration storage for the shared render context.

extern void* s_partyTaskTypeDescriptor;
extern void* s_partyMainTypeDescriptor;
extern float kFloat_6_2831855;
extern float s_partyMainFullTurnAngle;
extern float s_partyTaskFullTurnAngle;
extern float s_phfHelpersFullTurnAngle;
extern float s_courseSlotTableFullTurnAngle;
extern const char kPhfSignature[];
extern int strcmp(void*, const char*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void wmemcpy(void*, const void*, unsigned int);
extern void wmemmove(void*, const void*, unsigned int);
extern void strcpy(char*, const char*);
struct WStr;
extern WStr* WString_ReplaceRange(WStr*, unsigned int, unsigned int, const unsigned short*, const unsigned short*,
                                  const char*);
extern void __throw(const char*, void*, void (*)(void*));
extern void __dt__Q23std12out_of_rangeFv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
extern const char partyItemSelectStrings[];
extern unsigned short kWideStringReallocatedTerminator;
extern char __vt__10CPartyTask[];
extern int g_defaultActorSkipMask;
extern int g_actorSkipMask;
extern void __dt__7CVSMainFv(void*, int);
extern void __dl__FPv(void*);
extern void __destroy_new_array();
extern unsigned int strlen(const char*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
extern void appFreeUni__FPv(void*);
extern void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
extern void __dt__5CTaskFv(void*, int);
// The destructors take a delete flag, which the interface thunks pass through.
void* __dt__10CPartyMainFv(void*, int);
void* __dt__10CPartyTaskFv(void*, int);

struct LocalString {
    unsigned int a, b, c;
};
extern const char partyMainActorStrings[];

struct ManagerObject {
    char data[0x828];
};

struct WStr {
    unsigned int flags;
    unsigned int size;
    unsigned short* data;
};

struct ErrBlock {
    void* vtable;
    char* text;
    void* inner;
};

struct ErrText {
    char* text;
    char* allocator;
    ~ErrText() {
        if (text)
            delete[] text;
    }
};

struct ErrInner {
    void* vt;
    int a;
    int b;
    char* t;
    ErrInner(char* c) {
        a = 1;
        b = 1;
        vt = s_sharedCharDeleterVtable;
        t = c;
    }
};

#pragma use_lmw_stmw on
// Replace a UTF-16 range, preserving small-string storage, overlap handling, and exception cleanup.
// This is an inlined MSL basic_string<wchar_t>::replace(pos, n, first, last). In plain C:
//   if (pos > size) throw out_of_range; n = min(n, size - pos);
//   if (insert length or resulting length is too large) throw length_error;
//   newLen = size - n + (last - first);
//   if (newLen does not fit the current capacity) { allocate a bigger buffer (rounded up to a multiple of 8 chars,
//       doubling), copy head / inserted text / tail into it, free the old heap buffer, switch to heap storage; }
//   else { (copy [first,last) aside first if it overlaps the tail), slide the tail, copy the new text in place; }
// String layout: bit 31 of flags = heap storage. Short strings keep the length in byte 0 (7 bits) and 5 chars at +2.
// The two throw blocks build the exception object (message copy plus shared-pointer control block) exactly as
// MSL's inlined constructors do; they only exist to match the compiler and can be skipped when reading.
WStr* WString_ReplaceRange(WStr* self, unsigned int position, unsigned int eraseCount, const unsigned short* first,
                           const unsigned short* last, const char* allocator) {
    unsigned int usesHeap = self->flags >> 31; // bit 31 set: characters live in a heap buffer
    unsigned int capacity;
    unsigned int size;
    unsigned short* data;
    if (!usesHeap) {
        size = *reinterpret_cast<unsigned char*>(self) & 0x7F;
        data = reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(self) + 2);
        capacity = 5;
    } else {
        data = self->data;
        capacity = self->flags & 0x7FFFFFFF;
        size = self->size;
    }
    if (position > size) {
        // throw std::out_of_range(<message>)
        char dummy = 0;
        ErrBlock block;
        block.vtable = __vt__Q23std11logic_error;
        const char* msg = partyItemSelectStrings + kStringRangeError;
        unsigned int insertCount = strlen(msg) + 1;
        char* copy = static_cast<char*>(__nwa__FUl(insertCount * 2 - insertCount));
        block.text = copy;
        {
            ErrText et;
            et.text = copy;
            et.allocator = &dummy;
            ErrInner* inner = new ErrInner(copy);
            block.inner = inner;
            et.text = 0;
        }
        strcpy(block.text, msg);
        block.vtable = __vt__Q23std12out_of_range;
        __throw(partyItemSelectStrings + kStringRangeErrorType, &block, __dt__Q23std12out_of_rangeFv);
    }
    unsigned int remainingCount = size - position;
    const unsigned int* clampedEraseCount = &eraseCount;
    if (remainingCount < *clampedEraseCount)
        clampedEraseCount = &remainingCount;
    unsigned int removedCount = *clampedEraseCount;
    int insertCount = last - first;
    if (static_cast<unsigned int>(insertCount) > 0x7FFFFFFEu || size - removedCount > 0x7FFFFFFEu - insertCount) {
        // throw std::length_error(<message>)
        char dummy = 0;
        ErrBlock block;
        block.vtable = __vt__Q23std11logic_error;
        const char* msg = partyItemSelectStrings + kStringLengthError;
        unsigned int len = strlen(msg) + 1;
        char* copy = static_cast<char*>(__nwa__FUl(len * 2 - len));
        block.text = copy;
        {
            ErrText et;
            et.text = copy;
            et.allocator = &dummy;
            ErrInner* inner = new ErrInner(copy);
            block.inner = inner;
            et.text = 0;
        }
        strcpy(block.text, msg);
        block.vtable = __vt__Q23std12length_error;
        __throw(partyItemSelectStrings + kStringLengthErrorType, &block, __dt__Q23std12length_errorFv);
    }
    // Result length and number of characters after the erased range.
    unsigned int newLength = insertCount + (size - removedCount);
    unsigned int tailCount = size - (position + removedCount);
    if (newLength >= capacity) {
        // Grow: allocate a new buffer and rebuild head + inserted text + tail in it.
        capacity = (capacity + 7) & ~7u;
        unsigned int need = newLength + 1;
        while (capacity < need)
            capacity = (capacity * 2 + 7) & ~7u;
        unsigned int cap = capacity;
        unsigned short* out = static_cast<unsigned short*>(appMallocDummy__FUl(cap * 2));
        if (position)
            wmemcpy(out, data, position);
        unsigned short* mid = out + position;
        memmove(mid, first, (last - first) * 2);
        if (tailCount)
            wmemcpy(mid + insertCount, data + position + removedCount, tailCount);
        out[newLength] = kWideStringReallocatedTerminator;
        if (usesHeap)
            appFreeUni__FPv(data);
        else
            self->flags |= 0x80000000;
        self->flags = (self->flags & 0x80000000) | cap;
        self->data = out;
        self->size = newLength;
    } else {
        // Fits in place. If the source range overlaps the tail that will move, work from a temporary copy.
        WStr overlapCopy;
        overlapCopy.flags = 0;
        overlapCopy.size = 0;
        overlapCopy.data = 0;
        if (tailCount && data + position + insertCount < last && last <= data + size) {
            volatile char uninit;
            char overlapAllocatorTag = uninit;
            try {
                WString_ReplaceRange(&overlapCopy, 0, (overlapCopy.flags >> 31) ? 0 : (overlapCopy.flags & 0x7F), first,
                                     last, &overlapAllocatorTag);
            } catch (...) {
                if (overlapCopy.flags >> 31)
                    appFreeUni__FPv(overlapCopy.data);
                throw;
            }
            if (overlapCopy.flags >> 31)
                first = overlapCopy.data;
            else
                first = reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(&overlapCopy) + 2);
        }
        if (tailCount)
            wmemmove(data + position + insertCount, data + position + removedCount, tailCount);
        wmemmove(data + position, first, insertCount);
        data[newLength] = kWideStringInPlaceTerminator;
        if (!usesHeap)
            *reinterpret_cast<unsigned char*>(self) =
                (*reinterpret_cast<unsigned char*>(self) & 0x80) | (newLength & 0x7F);
        else
            self->size = newLength;
        if (overlapCopy.flags >> 31)
            appFreeUni__FPv(overlapCopy.data);
    }
    return self;
}

// Delete the previous character-count menu through its virtual destructor (vtable slot at +8).
static inline void DeleteSelectionScreen(CPartyMain* self) {
    void* previousMenu = self->selectionScreen;
    if (previousMenu != 0) {
        typedef void (*Dtor)(void*, int);
        Dtor dtor = *reinterpret_cast<Dtor*>(static_cast<char*>(*reinterpret_cast<void**>(previousMenu)) + 8);
        dtor(previousMenu, 1);
        self->selectionScreen = 0;
    }
}

// Forward party lifecycle messages or initialize the render context and character-count menu.
void CPartyMain::HandleMsg(void* message) {
    static_cast<CSharedDoc*>(g_sharedDoc)->gameMode = kPartyGameMode;
    int type = static_cast<PartyMainMessage*>(message)->type;
    // Messages 0/1 discard the current menu; 2/3 forward directly; 4 starts party setup.
    if (type < 0 || type > 4)
        return;
    if (type != kStartPartyMessage) {
        if (type != 2 && type != 3)
            DeleteSelectionScreen(this);
        ((CVSMain*)this)->HandleMsg((const CVSMainMessage*)message);
        return;
    }
    DeleteSelectionScreen(this);
    // Start party setup: lazily construct the global render context once (Sing_805DC648 holds the pointer
    // to it), reset its player records, then create the character-count menu.
    if (((int&)Sing_805DC648()) == 0) {
        if (s_renderContextInitialized == 0) {
            // One-time construction of s_renderContext: 4 player records, an int vector, 4 item records.
            char* base = s_renderContext;
            char* end = base + kRenderPlayerRecordsEnd;
            for (char* p = base; p < end; p += kRenderPlayerRecordSize)
                SaveData_InitPlayerRecord(p);
            ((IntVectorArray*)(end))->__ct();
            char* member = base + kRenderItemRecordsOffset;
            __construct_array(member, reinterpret_cast<void*>(PartyItemRecord_InitDefaults),
                              reinterpret_cast<void*>(PlayerRenderRecord_dtor), kRenderItemRecordSize,
                              kRenderItemRecordCount);
            reinterpret_cast<CEnemyRenderTable*>(member)->count = 0;
            RenderContext_ResetPlayerRecords(base);
            __register_global_object(s_renderContext, reinterpret_cast<void*>(__dt__13RenderContextFv),
                                     s_renderContextDtorRegistration_8008B524);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = reinterpret_cast<int>(s_renderContext);
    }
    RenderContext_ResetPlayerRecords(reinterpret_cast<void*>(((int&)Sing_805DC648())));
    int partyMode = static_cast<PartyMainMessage*>(message)->partyMode;
    static_cast<CSharedDoc*>(g_sharedDoc)->partyGameMode = partyMode;
    void* characterCountMenu = __nw__FUl(kCharaNumMenuSize);
    if (characterCountMenu != 0)
        ((CCharaNum*)characterCountMenu)->__ct();
    selectionScreen = characterCountMenu;
    state = 1; // 1: choosing the number of characters
    static_cast<CSharedDoc*>(g_sharedDoc)->golfRuleType = 0;
    strcpy(static_cast<CSharedDoc*>(g_sharedDoc)->playNameBuffer,
           static_cast<char*>(static_cast<CSharedDoc*>(g_sharedDoc)->strokePlayName));
}

// Create the party task's next actor request and install the returned actor.
void CPartyTask::CreateActors() {
    const char* strings = partyMainActorStrings;
    LocalString str = {0, 0, 0};
    unsigned int len = strlen(strings);
    String_ReserveCapacity(&str, len);
    // The allocator tag is ignored; an uninitialised byte is passed, read through a volatile local.
    volatile unsigned int uninit;
    char ch = *reinterpret_cast<volatile char*>(&uninit);
    String_ReplaceRange(&str, 0, 0, strings, strings + len, &ch);
    void* result =
        TaskActorTable_CreateNamedActor(this, strings + kPartyMainClassName, strings + kPartyMainActorName, 0, &str);
    ((CTask*)this)->SetMainActor(result);
    if ((str.a & 0x80000000) != 0)
        appFreeUni__FPv(reinterpret_cast<void*>(str.c));
}

// Check that a non-null file begins with the expected PHF signature.
static inline int PhfHeaderHasSignature(void* self) {
    if (self == 0)
        return 0;
    return !strcmp(self, kPhfSignature);
}

// Return the entry count only for a valid PHF header.
static inline int PhfHeaderEntryCount(void* self) {
    if (PhfHeaderHasSignature(self))
        return reinterpret_cast<PhfHeader*>(self)->entryCount;
    return 0;
}

// Bounds-check a PHF directory index and return its fixed-size entry.
static inline void* PhfCheckedEntryAt(void* self, int index) {
    if (!PhfHeaderHasSignature(self))
        return 0;
    if (index < 0 || index >= PhfHeaderEntryCount(self))
        return 0;
    return reinterpret_cast<PhfEntry*>(static_cast<char*>(self) + sizeof(PhfHeader)) + index;
}

// Return a PHF directory entry, or null for an invalid file or index.
void* PhfFile_GetEntry(void* self, int index) {
    void* result = PhfCheckedEntryAt(self, index);
    if (result == 0)
        return 0;
    return result;
}

// Check the PHF signature before resolving directory data.
static inline int PhfDataHasSignature(void* self) {
    if (self == 0)
        return 0;
    return !strcmp(self, kPhfSignature);
}

// Return a valid PHF header's directory count for data lookup.
static inline int PhfDataEntryCount(void* self) {
    if (PhfDataHasSignature(self))
        return reinterpret_cast<PhfHeader*>(self)->entryCount;
    return 0;
}

// Return a nonnegative directory entry within the validated entry count.
static inline void* PhfDataEntryAt(void* self, int index) {
    if (index >= 0) {
        if (index >= PhfDataEntryCount(self))
            return 0;
        return reinterpret_cast<PhfEntry*>(static_cast<char*>(self) + sizeof(PhfHeader)) + index;
    }
    return 0;
}

// Resolve entry data from the file's data base plus the entry's relative offset.
void* PhfFile_GetEntryData(void* self, int index) {
    if (!PhfDataHasSignature(self))
        return 0;
    void* result = PhfDataEntryAt(self, index);
    if (result == 0)
        return 0;
    unsigned int off = reinterpret_cast<PhfHeader*>(self)->dataOffset;
    unsigned int rel = reinterpret_cast<PhfEntry*>(result)->dataOffset;
    return static_cast<char*>(self) + off + rel;
}

// Return the party-main runtime type descriptor.
void* CPartyMain::GetRTTI() {
    return &s_partyMainTypeDescriptor;
}

// Adjust the task-main interface pointer before destroying party main.
void* CPartyMain::DestroyTaskMainInterface(int deleteFlag) {
    return __dt__10CPartyMainFv(reinterpret_cast<char*>(this) - 0x18, deleteFlag);
}

// Return the party-task runtime type descriptor.
void* CPartyTask::GetRTTI() {
    return &s_partyTaskTypeDescriptor;
}

// Adjust the tertiary interface pointer before calling the destructor.
void* CPartyTask::DestroyTertiaryInterface(int deleteFlag) {
    return __dt__10CPartyTaskFv(reinterpret_cast<char*>(this) - 8, deleteFlag);
}

// Compare the file contents with the PHF signature (not a path suffix).
int BpObject_HasPhfSignature(void* self) {
    if (self == 0) {
        return 0;
    }
    return !strcmp(self, kPhfSignature);
}

// Return a valid PHF file's entry-count word through the original pointer-sized return type.
void* PhfFile_GetEntryCountWord(void* self) {
    int valid;
    if (self == 0)
        valid = 0;
    else
        valid = !strcmp(self, kPhfSignature);
    if (valid)
        return reinterpret_cast<void*>(reinterpret_cast<PhfHeader*>(self)->entryCount);
    return 0;
}

// Destroy the task base; the compiler adds the delete handling for this destructor.
CPartyTask::~CPartyTask() {
    __dt__5CTaskFv(this, 0);
}

// Initialize an empty course-slot table with no selected slot or storage.
void CourseSlotTable_InitEmpty(void* self) {
    unsigned int zero = 0;
    unsigned int full = 0xFFFFFFFF;
    reinterpret_cast<CourseSlotTable*>(self)->entries = reinterpret_cast<CourseSlotEntry*>(zero);
    reinterpret_cast<CourseSlotTable*>(self)->selectedIndex = static_cast<unsigned short>(full);
    reinterpret_cast<CourseSlotTable*>(self)->storage = reinterpret_cast<void*>(zero);
    // The original branches over unreachable array-cleanup code after these initializers.
}

// Destroy the versus-main base; the compiler adds the delete handling for this destructor.
CPartyMain::~CPartyMain() {
    __dt__7CVSMainFv(this, 0);
}

// Initialize the first cached full-turn angle in this unit.
void InitPartyMainFullTurnAngle() {
    s_partyMainFullTurnAngle = kFloat_6_2831855;
}

// Initialize the second cached full-turn angle in this unit.
void InitPartyTaskFullTurnAngle() {
    s_partyTaskFullTurnAngle = kFloat_6_2831855;
}

// Initialize the third cached full-turn angle in this unit.
void InitPhfHelpersFullTurnAngle() {
    s_phfHelpersFullTurnAngle = kFloat_6_2831855;
}

// Initialize the fourth cached full-turn angle in this unit.
void InitCourseSlotTableFullTurnAngle() {
    s_courseSlotTableFullTurnAngle = kFloat_6_2831855;
}

// Test whether a course-slot number belongs to the extra-slot range.
int CourseSlotTable_IsExtraSlotRange(void*, unsigned int value) {
    return value >= (unsigned int)kExtraCourseSlotBase;
}

// Return one of five fixed-size course slots when storage exists.
void* SlotTable_GetEntry(void* self, unsigned int index) {
    void* items = reinterpret_cast<CourseSlotTable*>(self)->entries;
    if (items == 0 || index >= kCourseSlotCount) {
        return 0;
    }
    return static_cast<CourseSlotEntry*>(items) + index;
}

// Narrow the slot index to one byte before looking up one of five course slots.
void* SlotTable_GetEntryByte(void* self, unsigned int index) {
    void* items = reinterpret_cast<CourseSlotTable*>(self)->entries;
    index = static_cast<unsigned char>(index);
    if (items == 0 || index >= kCourseSlotCount) {
        return 0;
    }
    return static_cast<CourseSlotEntry*>(items) + index;
}

// Map an extra-slot index through the byte-sized base offset before its bounds check.
void* CourseSlotTable_GetExtraEntry(void* self, unsigned char index) {
    void* items = reinterpret_cast<CourseSlotTable*>(self)->entries;
    // Assignment back to unsigned char intentionally wraps before checking the five-slot range.
    index = index + kExtraCourseSlotBase;
    if (items == 0 || index >= kCourseSlotCount) {
        return 0;
    }
    return static_cast<CourseSlotEntry*>(items) + index;
}

// Initialize the party task's three interfaces and default actor skip masks.
CPartyTask::CPartyTask() {
    ((CTask*)this)->Init();
    vtbl = __vt__10CPartyTask;
    secondaryVtbl = __vt__10CPartyTask + 0x10;
    tertiaryVtbl = __vt__10CPartyTask + 0x1C;
    g_defaultActorSkipMask = kDefaultActorSkipMask;
    g_actorSkipMask = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462040", _ctors_u8008B524_0, InitPartyMainFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU80462040", _ctors_u8008B524_1, InitPartyTaskFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU80462040", _ctors_u8008B524_2, InitPhfHelpersFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU80462040", _ctors_u8008B524_3, InitCourseSlotTableFullTurnAngle);
#pragma pop

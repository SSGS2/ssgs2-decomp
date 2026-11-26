// In-game dialog base (CInGameDialog) and the saved player snapshot: the open and close transitions
// of dialogs with their wide-text messages, and the copy of each player's stats, equipment and
// caddie choice into the saved record.
//
// Also holds the byte-vector and allocation helpers used by the message and text code.

#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CPangyaMsgDrawObject.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "classes/CInGameDialog.h"
#include "classes/CGolfDoc.h"

#define AsCInGameDialog(p) ((CInGameDialog*)(p))
extern "C" {

extern float kFloat_6_2831855;
extern float s_dialogFullTurn;
extern float kFloat_0_805E0328;
extern float kFloat_0_5_TextureDraw;
extern float kFloat_200_805E0330;
extern float kFloat_1_PartyBalloon;
extern void* g_soundManager;
extern CSharedDoc* g_sharedDoc;
struct DialogSoundPool {
    char openSound[15];
    char closeSound[16];
    char otherStrings[169];
};
extern DialogSoundPool dialogSoundAndErrorStrings;
struct ProfileStringPool {
    char defaultNameFormat[9];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char padding[4];
};
extern ProfileStringPool playerProfileDefaultNameStrings;
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);

void* Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void WString_ReplaceRange(void*, int, int, int, int, unsigned char*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void IUnknownCustom_dtor(void*, int);
extern int __vt__13CInGameDialog[];

enum DialogState {
    Dialog_Reset = 1,
    Dialog_Inactive = 2,
    Dialog_Opening = 3,
    Dialog_Open = 4,
    Dialog_Closing = 5
};
enum {
    SaveCounterMaximum = 99999999,
    SaveShortCounterMaximum = 9999,
    GameMode_Stroke = 0,
    GameMode_Party = 6
};
enum SnapshotLayout {
    kSnapshotStride = 0x1D4,
    kSnapshotPlayerSlots = 4,
    kSnapshotProfileBytes = 0x20,
    kSnapshotCourseRecordBytes = 0x70
};

struct InGameDialogView {
    int refcount;
    void** vtable;
    // Constructor clears these four floats; no reader here establishes axes or size/position order.
    float x08, x0c, x10, x14;
    int dialogState;
    float transitionDuration, elapsedTime, transitionProgress, autoCloseDelay;
    // Initial request state is closed (open=0, close=1); +34/+38 are constructor-only zeros here.
    // ResetDialog clears timers/text, but does not reset these four flags.
    int openRequested, closeRequested, resetState34, blockRegistryClose;
    void* messageDraw;
    union {
        void* stringStorageWord;
        struct {
            unsigned short storageTag;
            unsigned short inlineText[1];
        } shortString;
    };
    int stringLength;
    void* stringData;
};

// Reset transition state and erase all text from the inline or heap-backed wide string.
// Close the dialog, release its message renderer and wide string, and optionally delete it.
int __dt__15SmallDialogBaseFv(InGameDialogView*, int);

#define SAVE_FIELD_OFFSET(field) ((int)&((SavedSnapshotFields*)0)->field)

// Save fields transferred to/from the live player snapshot; statistics stay in their on-save order.
// The eight counters and three floating-point statistics are not individually identified yet.
// Persistence schema (saved offset -> live offset, relative to the rebased view):
// counters[0..3] +1dc..1e8 -> +4c..58, 32-bit values clamped to [0,99999999];
// counters[4..5] +1ec/+1f0 -> +5c/+5e, narrowed to shorts, saved in [0,9999];
// counters[6..7] +1f4/+1f8 -> +60/+64, extraCounters +1fc/+200 -> +68/+6c;
// floatStatistics +204/+208/+20c -> +70/+74/+78, clamped to [0,100000000];
// finalCounter +210 -> +7c uses the integer clamp; lastFloatStatistic +214 -> +80
// is copied WITHOUT a clamp. Gameplay identities/units remain unresolved. Preserve
// slot order and widths rather than treating these arrays as interchangeable totals.
// Editing guide: each named array denotes a fixed saved-offset range above,
// not a sum to recompute. No observed writer here identifies strokes, wins,
// distances, percentages or their units. Establish those consumers before
// changing a statistic formula. NaN floats pass the comparison-based clamp.
struct SavedSnapshotFields {
    char profilePrefix[0x24];
    int pangBits;       // Numeric pang balance; never dereferenced as a pointer.
    int experienceBits; // Numeric experience value, copied with signed word loads/stores.
    char selectionPrefix[5];
    char equippedItemIndices[8];
    char inventoryAndEquipment[0xF9];
    short lastItemCount;
    short reservedItemCount;
    char auxPartFlags[7];
    char statisticsPrefix[0x9F];
    int counters[8];
    int extraCounters[2];
    float floatStatistics[3];
    int finalCounter;
    float lastFloatStatistic;
    signed char courseScores[0x44];
    int courseRecordsStart;
};
// This view begins at CSharedDoc, before its four 0x1D4-byte snapshots at +0x10.
struct SnapshotProfileHeader {
    char profilePrefix[0x20];
    char auxPartFlags[7];
    char alignmentPadding;
    int reservedWord;
    int selectedCourseScore;
    short reservedItemCount;
    short lastItemCount;
};
struct SharedPlayerSnapshotView {
    char prefix[0x10];
    char profilePrefix[0x20];
    char auxPartFlags[7];
    char alignmentPadding;
    int reservedWord;
    int selectedCourseScore;
    short reservedItemCount;
    short lastItemCount;
    int pangBits;       // Numeric pang balance; never dereferenced as a pointer.
    int experienceBits; // Numeric experience value, copied with signed word loads/stores.
    int counters[4];
    short narrowedCounters[2];
    int remainingCounters[4];
    float floatStatistics[3];
    int finalCounter;
    float lastFloatStatistic;
    char equipmentPrefix[8];
    int equippedItemIds[8];
    char equipmentPadding[4];
    char courseRecords[0x70];
    char playerState[0x98];
    int caddieId;
    char caddieState[8];
    int caddieValue;
    char tail[0xC];
};
struct SavedCaddieSnapshot {
    unsigned char flags;
    char padding[3];
    int value;
};
struct SavedCaddieSnapshotView {
    char prefix[0x46C];
    SavedCaddieSnapshot caddies[8];
};
#define SNAPSHOT_OFFSET(f, o) \
    typedef char Snapshot_offset_##f[((int)&((SharedPlayerSnapshotView*)0)->f == (o)) ? 1 : -1]
SNAPSHOT_OFFSET(pangBits, 0x44);
SNAPSHOT_OFFSET(counters, 0x4C);
SNAPSHOT_OFFSET(equippedItemIds, 0x8C);
SNAPSHOT_OFFSET(courseRecords, 0xB0);
SNAPSHOT_OFFSET(caddieId, 0x1B8);
SNAPSHOT_OFFSET(caddieValue, 0x1C4);
#undef SNAPSHOT_OFFSET
typedef char Snapshot_stride[(sizeof(SharedPlayerSnapshotView) == 0x1D4) ? 1 : -1];
typedef char SavedSnapshot_counters[((int)&((SavedSnapshotFields*)0)->counters == 0x1DC) ? 1 : -1];

int ItemInventory_FindCaddieIndex(void*, void*);
int ItemInventory_FindItemIndex(void*, int);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* SaveRecord_CopyCtor(void*, void*);
void* SaveData_GetCurrentRecord(void*);
void SaveData_SetCurrentRecord(void*, void*);
void SaveData_Ctor(const char*);
void* SaveRecord_CopyCtor(void*, void*);
extern char s_saveDataInitialized;
extern const char s_saveData[];
extern float kFloat_0_805E0320;
extern float kFloat_100000000_805E0324;

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
struct GolfPlayerEntryView {
    void* player;
    int entryKind;
};

// Keep these expression macros: inline functions change this writer's machine code.
// Inputs must be side-effect-free lvalues; each successful branch may read again.
// Integer clamps use the persistence schema above; unordered floats pass through.
#define CLAMP_INT(v) ((v) < 0 ? 0 : ((v) > SaveCounterMaximum ? SaveCounterMaximum : (v)))
#define CLAMP_SHORT(v) ((v) < 0 ? 0 : ((v) > SaveShortCounterMaximum ? SaveShortCounterMaximum : (v)))
#define CLAMP_FLOAT(f) \
    ((f) < kFloat_0_805E0320 ? kFloat_0_805E0320 : ((f) > kFloat_100000000_805E0324 ? kFloat_100000000_805E0324 : (f)))
// Copy fixed statistic slots using the persistence clamps; leave other fields alone.
// Both views are borrowed; slot identities/units remain as documented above.
static inline void CopyClampedSnapshotStatistics(SavedSnapshotFields* saved, SharedPlayerSnapshotView* live) {
    saved->counters[0] = CLAMP_INT(live->counters[0]);
    saved->counters[1] = CLAMP_INT(live->counters[1]);
    saved->counters[2] = CLAMP_INT(live->counters[2]);
    saved->counters[3] = CLAMP_INT(live->counters[3]);
    saved->counters[4] = (short)CLAMP_SHORT(live->narrowedCounters[0]);
    saved->counters[5] = (short)CLAMP_SHORT(live->narrowedCounters[1]);
    saved->counters[6] = CLAMP_INT(live->remainingCounters[0]);
    saved->counters[7] = CLAMP_INT(live->remainingCounters[1]);
    saved->extraCounters[0] = CLAMP_INT(live->remainingCounters[2]);
    saved->extraCounters[1] = CLAMP_INT(live->remainingCounters[3]);
    saved->floatStatistics[0] = CLAMP_FLOAT(live->floatStatistics[0]);
    saved->floatStatistics[1] = CLAMP_FLOAT(live->floatStatistics[1]);
    saved->floatStatistics[2] = CLAMP_FLOAT(live->floatStatistics[2]);
    saved->finalCounter = CLAMP_INT(live->finalCounter);
    saved->lastFloatStatistic = live->lastFloatStatistic;
}

// Translate eight live IFF IDs into saved inventory indices; neither input owns data.
// Each lookup can return the inventory API's absent sentinel, narrowed as before.
// Cursor and slot references retain the caller's loop-variable lifetimes; this
// keeps the original register allocation while sharing the three identical loops.
static inline void CopySnapshotEquipmentSelection(char* savedRecord, char*& equippedItemCursor, int& equipmentSlot) {
    equipmentSlot = 0;
    do {
        int itemTypeId = *(int*)(equippedItemCursor + 0x08);
        int savedItemIndex = ItemInventory_FindItemIndex(g_sharedDoc->itemInventory, itemTypeId);
        savedRecord[equipmentSlot + SAVE_FIELD_OFFSET(equippedItemIndices)] = (char)savedItemIndex;
        ++equipmentSlot;
        equippedItemCursor += 4;
    } while (equipmentSlot < 8);
}

// Copy the selected caddie's value only when a caddie is equipped.
static inline void CopySnapshotCaddieValue(char* caddieRecords, char* liveSnapshot) {
    if (((SharedPlayerSnapshotView*)liveSnapshot)->caddieId != 0) {
        int slot = ItemInventory_FindCaddieIndex(g_sharedDoc->itemInventory,
                                                 (void*)((SharedPlayerSnapshotView*)liveSnapshot)->caddieId);
        ((SavedCaddieSnapshot*)caddieRecords)[slot].value = ((SharedPlayerSnapshotView*)liveSnapshot)->caddieValue;
    }
}

// Persist the lowest-priority local player snapshot, applying mode-specific fields and save clamps.
// Copies the current save record to a stack temporary before modifying it; the
// final SetCurrentRecord copies it into the selected save slot; it does
// not retain or free this stack pointer (save_data_records_and_defaults.cpp:1173).
// Stroke writes equipment, all statistics and course/caddie records. Party mode
// variant 0 writes the statistics/equipment subset; other party variants preserve
// most statistics and currency. Other modes republish the unchanged copied record.
// Field policy ("yes" means this routine replaces the copied save value):
//                         Stroke   Party variant 0   Party other   Other modes
// experience/equipment     yes          yes              yes            no
// pang/clamped statistics  yes          yes               no            no
// lastFloatStatistic       yes          yes              yes            no
// lastItemCount/aux flags  yes          yes              yes            no
// reservedItemCount/score  yes           no               no            no
// course/caddie records    yes       caddie only           no            no
// All writes target the stack copy until SetCurrentRecord publishes it.
void SaveData_PersistLocalPlayerSnapshot() {
    int playerIndex;
    char* liveCourseRecords;
    int equipmentSlot;
    char* equippedItemCursor;
    int itemIndex;
    char* caddieRecords;
    char* savedCourseRecords;
    char* savedRecord;
    char* liveSnapshot;
    int unusedScratch;

    // Phase 1: select a local (entryKind==0) player and copy the existing save.
    if (!(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
        return;
    playerIndex = ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->SelectPlayerBySnapshotWords();
    if (playerIndex < 0)
        return;
    if (!GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)playerIndex))
        return;
    if (((GolfPlayerEntryView*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                          (unsigned char)playerIndex))
            ->entryKind)
        return;
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)s_saveData;
    }
    void* source = SaveData_GetCurrentRecord(GetSaveDataSlot());
    char buffer[0x828];
    SaveRecord_CopyCtor(buffer, source);
    savedRecord = buffer;
    savedCourseRecords = (char*)&((SavedSnapshotFields*)buffer)->courseRecordsStart;
    caddieRecords = (char*)((SavedCaddieSnapshotView*)buffer)->caddies;
    // Phase 2: borrow the live snapshot and its course-record array.
    // Equipment loops below use inventory indices; course records remain bytes.
    int mode = (*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode);
    liveSnapshot = (char*)g_sharedDoc + playerIndex * kSnapshotStride;
    equippedItemCursor = (char*)&((SharedPlayerSnapshotView*)liveSnapshot)->equipmentPrefix;
    liveCourseRecords = ((SharedPlayerSnapshotView*)liveSnapshot)->courseRecords;

    // Stroke: currency + equipment + clamped statistics + course/caddie records.
    if (mode == GameMode_Stroke) {
        int pangValue = ((SharedPlayerSnapshotView*)liveSnapshot)->pangBits;
        int experienceValue = ((SharedPlayerSnapshotView*)liveSnapshot)->experienceBits;
        ((SavedSnapshotFields*)savedRecord)->pangBits = pangValue;
        ((SavedSnapshotFields*)savedRecord)->experienceBits = experienceValue;
        memcpy(((SavedSnapshotFields*)savedRecord)->auxPartFlags,
               ((SharedPlayerSnapshotView*)liveSnapshot)->auxPartFlags, 7);
        CopySnapshotEquipmentSelection(savedRecord, equippedItemCursor, itemIndex);
        int countValue = ((SharedPlayerSnapshotView*)liveSnapshot)->reservedItemCount;
        ((SavedSnapshotFields*)savedRecord)->reservedItemCount = (short)countValue;
        countValue = ((SharedPlayerSnapshotView*)liveSnapshot)->lastItemCount;
        ((SavedSnapshotFields*)savedRecord)->lastItemCount = (short)countValue;
        int courseRecordIndex = (*(int*)&AsCSharedDoc(g_sharedDoc)->courseRecordIndex);
        savedRecord[courseRecordIndex + SAVE_FIELD_OFFSET(courseScores)] =
            ((SharedPlayerSnapshotView*)liveSnapshot)->selectedCourseScore;
        CopyClampedSnapshotStatistics((SavedSnapshotFields*)savedRecord, (SharedPlayerSnapshotView*)liveSnapshot);
        memcpy(savedCourseRecords, liveCourseRecords, kSnapshotCourseRecordBytes);
        CopySnapshotCaddieValue(caddieRecords, liveSnapshot);
    } else if (mode == GameMode_Party) {
        int partyVariant = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode);
        // Variant 0 shares the statistics schema but preserves course records
        // and reservedItemCount. The other variants also preserve pang/totals.
        if (partyVariant == 0) {
            int pangValue = ((SharedPlayerSnapshotView*)liveSnapshot)->pangBits;
            int experienceValue = ((SharedPlayerSnapshotView*)liveSnapshot)->experienceBits;
            ((SavedSnapshotFields*)savedRecord)->pangBits = pangValue;
            ((SavedSnapshotFields*)savedRecord)->experienceBits = experienceValue;
            memcpy(((SavedSnapshotFields*)savedRecord)->auxPartFlags,
                   ((SharedPlayerSnapshotView*)liveSnapshot)->auxPartFlags, 7);
            CopySnapshotEquipmentSelection(savedRecord, equippedItemCursor, equipmentSlot);
            int countValue = ((SharedPlayerSnapshotView*)liveSnapshot)->lastItemCount;
            ((SavedSnapshotFields*)savedRecord)->lastItemCount = (short)countValue;
            CopyClampedSnapshotStatistics((SavedSnapshotFields*)savedRecord, (SharedPlayerSnapshotView*)liveSnapshot);
            CopySnapshotCaddieValue(caddieRecords, liveSnapshot);
        } else {
            int experienceValue = ((SharedPlayerSnapshotView*)liveSnapshot)->experienceBits;
            ((SavedSnapshotFields*)savedRecord)->experienceBits = experienceValue;
            memcpy(((SavedSnapshotFields*)savedRecord)->auxPartFlags,
                   ((SharedPlayerSnapshotView*)liveSnapshot)->auxPartFlags, 7);
            CopySnapshotEquipmentSelection(savedRecord, equippedItemCursor, equipmentSlot);
            int countValue = ((SharedPlayerSnapshotView*)liveSnapshot)->lastItemCount;
            ((SavedSnapshotFields*)savedRecord)->lastItemCount = (short)countValue;
            ((SavedSnapshotFields*)savedRecord)->lastFloatStatistic =
                ((SharedPlayerSnapshotView*)liveSnapshot)->lastFloatStatistic;
        }
    }

    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)s_saveData;
    }
    // Phase 3: publish only after every mode-specific update to the copy is complete.
    SaveData_SetCurrentRecord(GetSaveDataSlot(), savedRecord);
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_common_subs reset

struct CharVec80254 {
    unsigned char* data;
    unsigned int size;
    unsigned int capacity;
};

extern void* __vt__Q23std11logic_error[];
extern void* s_sharedCharDeleterVtable[];
extern void* __vt__Q23std12length_error[];
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __throw(void*, void*, void*) __attribute__((noreturn));
typedef void (*ThrowFn80254BD4)(void*, void*, void*);
extern void __dt__Q23std12length_errorFv(void*);

struct LogicError80254 {
    void* vt;
    char* msg;
    void* rep;
};

// Shared exception-message control block, matching std::detail::message_control.
// refs (+4) is the strong count; weakCount (+8) is the weak/control count, NOT
// string capacity. Both start at one; data (+C) owns a separate char array.
struct MessageControlBlock {
    void* vt;
    int refs;
    int weakCount;
    char* data;
    // Retain the message array in a newly constructed shared deleter control block.
    MessageControlBlock(char* d) {
        refs = 1;
        weakCount = 1;
        vt = (void*)s_sharedCharDeleterVtable;
        data = d;
    }
};

struct StringTemp80254 {
    char* data;
    char* allocRef;
};

struct MessageArrayGuard {
    char* data;
    char* allocRef;
    // Free a message array until the exception's shared owner takes it.
    ~MessageArrayGuard() {
        if (data)
            __dla__FPv(data);
    }
};

namespace std {
struct exception {
    void* vt;
    char* msg;
    void* rep;
    ~exception();
};
}
typedef std::exception ErrLocal802558EC;

struct AllocTemp802558EC {
    char c;
    // Initialize the allocator marker used by the insertion exception payload.
    AllocTemp802558EC() {
        c = 0;
    }
};

// Borrow the allocator marker for the message-array guard's lifetime.
inline void setref802558EC(MessageArrayGuard& s, const AllocTemp802558EC& a) {
    s.allocRef = (char*)&a.c;
}

struct Guard80254 {
    // ptr owns replacement storage until commit, then owns the old buffer to free.
    // count is constructed byte count; cap is allocated bytes; capRef borrows the
    // destination capacity word; copyCursor is the insertion/copy cursor, not final size.
    char* ptr;
    unsigned int count;
    unsigned int cap;
    void* capRef;
    unsigned int copyCursor;
    // Release the owned buffer, either uncommitted replacement or superseded storage.
    ~Guard80254() {
        if (ptr) {
            count = 0;
            appFreeUni__FPv(ptr);
        }
    }
};

struct StrLocal80254BD4 {
    char* data;
    char* allocRef;
    // Free an exception message unless construction transferred it to MessageControlBlock.
    ~StrLocal80254BD4() {
        if (data)
            __dla__FPv(data);
    }
};

struct Alloc80254BD4 {
    char c;
    char pad[3];
};

// Select the larger capacity increment by reference, preserving caller temporary lifetimes.
inline const unsigned int& max80254BD4(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Count bytes in the valid [first,last) range from the same allocation.
inline unsigned int distance80254BD4(unsigned char* first, unsigned char* last) {
    return (unsigned int)(last - first);
}

#define THROW_VECTOR_LENGTH_ERROR_80254BD4(N, Z)                       \
    char* text##N;                                                     \
    char* buf##N;                                                      \
    volatile LogicError80254* ep##N;                                   \
    flag##N.c = Z;                                                     \
    text##N = playerProfileDefaultNameStrings.vectorLengthError;       \
    ep##N = &error##N;                                                 \
    error##N.vt = (void*)__vt__Q23std11logic_error;                    \
    unsigned int n##N = strlen(text##N) + 1;                           \
    buf##N = (char*)__nwa__FUl(n##N * 2 - n##N);                       \
    error##N.msg = buf##N;                                             \
    {                                                                  \
        StrLocal80254BD4 str##N;                                       \
        str##N.data = buf##N;                                          \
        str##N.allocRef = &flag##N.c;                                  \
        MessageControlBlock* rep##N = new MessageControlBlock(buf##N); \
        error##N.rep = rep##N;                                         \
        str##N.data = 0;                                               \
    }                                                                  \
    strcpy(error##N.msg, text##N);                                     \
    error##N.vt = (void*)__vt__Q23std12length_error;                   \
    __throw(playerProfileDefaultNameStrings.lengthErrorType, (void*)ep##N, (void*)__dt__Q23std12length_errorFv)

#define THROW_LENGTH_ERROR_802558EC(N)                                 \
    AllocTemp802558EC alloc##N;                                        \
    ErrLocal802558EC error##N;                                         \
    ErrLocal802558EC* ep##N = &error##N;                               \
    char* text##N = playerProfileDefaultNameStrings.vectorLengthError; \
    ep##N->vt = (void*)__vt__Q23std11logic_error;                      \
    unsigned int n##N = strlen(text##N) + 1;                           \
    char* buf##N = (char*)__nwa__FUl(n##N * 2 - n##N);                 \
    ep##N->msg = buf##N;                                               \
    {                                                                  \
        MessageArrayGuard str##N;                                      \
        str##N.data = buf##N;                                          \
        setref802558EC(str##N, alloc##N);                              \
        MessageControlBlock* rep##N = new MessageControlBlock(buf##N); \
        ep##N->rep = rep##N;                                           \
        str##N.data = 0;                                               \
    }                                                                  \
    strcpy(ep##N->msg, text##N);                                       \
    ep##N->vt = (void*)__vt__Q23std12length_error;                     \
    __throw(playerProfileDefaultNameStrings.lengthErrorType, (void*)ep##N, (void*)__dt__Q23std12length_errorFv)

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Append one byte, growing vector storage with overflow checks and exception-safe cleanup.
// value borrows one readable byte and may point inside self->data: on growth it
// is read into the new tail before the old allocation is cleared or released.
// On success size increases by one; the vector owns data and the guard frees only
// superseded storage. Capacity overflow throws length_error rather than wrapping.
void ByteVector_AppendOne(CharVec80254* self, const unsigned char* value) {
    unsigned int cap;
    unsigned int growth;
    unsigned int diff;
    unsigned int half;
    unsigned int third;
    Alloc80254BD4 flag2;
    Alloc80254BD4 flag1;
    volatile LogicError80254 error2;
    volatile LogicError80254 error1;
    unsigned int size = self->size;
    unsigned int capacity = self->capacity;
    // Phase 1: append directly if spare storage exists; otherwise validate growth.
    if (size < capacity) {
        self->size = self->size + 1;
        self->data[self->size - 1] = *value;
        return;
    }
    if ((unsigned int)(-1 - self->capacity) < 1U) {
        THROW_VECTOR_LENGTH_ERROR_80254BD4(1, 0);
    }
    // Phase 2: guard the replacement and choose capacity (3/5, then 1/2 growth,
    // saturating at UINT_MAX as multiplication becomes unsafe).
    Guard80254 g;
    // Alias the insertion/copy cursor without changing its lifetime.
    unsigned int& copyCursor = g.copyCursor;
    void* capRef;
    unsigned int guardZero;
    guardZero = 0;
    g.ptr = (char*)guardZero;
    diff = (self->size + 1) - self->capacity;
    capRef = &self->capacity;
    g.count = guardZero;
    cap = self->capacity;
    g.cap = guardZero;
    g.capRef = capRef;
    copyCursor = guardZero;
    if (diff > (unsigned int)(-1 - cap)) {
        THROW_VECTOR_LENGTH_ERROR_80254BD4(2, guardZero);
    }
    if (cap < 0x55555555u) {
        unsigned int c = cap + 1;
        third = (c * 4 - c) / 5;
        growth = cap + max80254BD4(third, diff);
    } else if (cap < 0xAAAAAAAAu) {
        half = (cap + 1) / 2;
        growth = cap + max80254BD4(half, diff);
    } else {
        growth = 0xFFFFFFFFu;
    }
    // Phase 3: construct the appended byte first, then transfer the old prefix.
    unsigned char* fresh = (unsigned char*)appMallocDummy__FUl(growth);
    unsigned int oldSize = self->size;
    g.ptr = (char*)fresh;
    g.cap = growth;
    copyCursor = oldSize;
    memset(g.count + (fresh + oldSize), *value, 1);
    unsigned int n;
    unsigned char* old = self->data;
    n = distance80254BD4(old, old + self->size);
    g.count += 1;
    copyCursor = copyCursor - n;
    memcpy(g.ptr + copyCursor, old, n);
    memset(old, 0, n);
    unsigned char* cur;
    unsigned int newCap;
    unsigned int newSize;
    unsigned int prevCap;
    unsigned char* newData;
    newSize = g.count + n;
    g.count = newSize;
    // Phase 4: commit the new buffer/size/capacity; g now frees the cleared old buffer.
    cur = self->data;
    newData = (unsigned char*)g.ptr;
    prevCap = self->capacity;
    newCap = g.cap;
    self->size = 0;
    g.ptr = (char*)cur;
    self->capacity = newCap;
    g.cap = prevCap;
    self->data = newData;
    self->size = newSize;
    g.count = 0;
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_common_subs reset

// Clamp the player slot and copy save equipment/statistics into its live shared-document snapshot.
// p borrows a complete saved record, including the caddie table at +46c. This
// function copies values and bytes; pangBits/experienceBits are numeric words,
// not owned allocations. Its legacy int return has no defined value on this path.
int SharedDoc_CopyPlayerSnapshot(const SavedSnapshotFields* p, int index) {
    int bounded;
    if (index < 0)
        bounded = 0;
    else if (index > 3)
        bounded = 3;
    else
        bounded = index;
    // Phase 1: the view includes a +10 prefix. Rebase by slot stride only; adding
    // another +10 here would shift every destination field past its correct slot.
    SharedPlayerSnapshotView* record = (SharedPlayerSnapshotView*)((char*)g_sharedDoc + bounded * kSnapshotStride);
    char* buffer = record->profilePrefix;
    char* tail = record->courseRecords;
    memcpy(buffer, p, kSnapshotProfileBytes);
    memcpy(((SnapshotProfileHeader*)buffer)->auxPartFlags, p->auxPartFlags, 7);
    ((SnapshotProfileHeader*)buffer)->reservedWord = 0;
    // Phase 2: only Stroke mode replaces the selected course score/reserved count.
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == GameMode_Stroke) {
        int base = (*(int*)&AsCSharedDoc(g_sharedDoc)->courseRecordIndex);
        short v134 = p->reservedItemCount;
        ((SnapshotProfileHeader*)buffer)->selectedCourseScore = p->courseScores[base];
        ((SnapshotProfileHeader*)buffer)->reservedItemCount = v134;
    }
    ((SnapshotProfileHeader*)buffer)->lastItemCount = p->lastItemCount;
    // Phase 3: copy the schema above without save-time clamps, then caddie/course data.
    record->pangBits = p->pangBits;
    record->experienceBits = p->experienceBits;
    record->counters[0] = p->counters[0];
    record->counters[1] = p->counters[1];
    record->counters[2] = p->counters[2];
    record->counters[3] = p->counters[3];
    record->narrowedCounters[0] = p->counters[4];
    record->narrowedCounters[1] = p->counters[5];
    record->remainingCounters[0] = p->counters[6];
    record->remainingCounters[1] = p->counters[7];
    record->remainingCounters[2] = p->extraCounters[0];
    record->remainingCounters[3] = p->extraCounters[1];
    record->floatStatistics[0] = p->floatStatistics[0];
    record->floatStatistics[1] = p->floatStatistics[1];
    record->floatStatistics[2] = p->floatStatistics[2];
    record->finalCounter = p->finalCounter;
    record->lastFloatStatistic = p->lastFloatStatistic;
    if (record->caddieId != 0) {
        int slot = ItemInventory_FindCaddieIndex(g_sharedDoc->itemInventory, (void*)record->caddieId);
        record->caddieValue = ((const SavedCaddieSnapshotView*)p)->caddies[slot].value;
    }
    memcpy(tail, &p->courseRecordsStart, kSnapshotCourseRecordBytes);
    // The original returns no value: its callers never read one.
}

// Dialog virtual contract (byte offsets from the vtable start):
// +10 request-close(animate); +1C reset/setup; +20 complete-close;
// +24 can-open -> bool; +28 can-close -> bool; +30 complete-open;
// +34 start-open; +38 start-close; +3C frame-hook; +40 ready -> bool.
// DialogCloseInterface/DialogOpenInterface include the four-byte ownership prefix before the vptr; retain
// their placeholder slots until the methods are identified.
enum DialogVirtualSlot {
    kRequestCloseSlot = 4,
    kDialogSetupSlot = 7,
    kCompleteCloseSlot = 8,
    kCompleteOpenSlot = 12,
    kFrameHookSlot = 15,
    kReadySlot = 16
};
struct DialogOpenOwnerPrefix {
    char pad[4];
};
class DialogOpenInterface : public DialogOpenOwnerPrefix {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual int v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
};
// Query the derived opening prerequisite (vtable +24); nonzero allows opening.
static inline int DialogAllowsOpening(InGameDialogView* dialog) {
    return ((DialogOpenInterface*)dialog)->v24();
}
// Dispatch the derived start-open hook (+34); it may select opening or complete it.
static inline void DialogStartOpening(InGameDialogView* dialog) {
    ((DialogOpenInterface*)dialog)->v34();
}
// Dispatch the derived complete-open hook (+30), bypassing animation.
static inline void DialogCompleteOpening(InGameDialogView* dialog) {
    ((DialogOpenInterface*)dialog)->v30();
}
// Request opening; return zero if already requested or vetoed by can-open.
// animate!=0 starts opening; zero completes opening immediately.
int CInGameDialog::RequestOpen(InGameDialogView* p, int animate) {
    if (p->openRequested != 0)
        return 0;
    int result = DialogAllowsOpening(p);
    if (result != 0) {
        Sound_PlayByName(g_soundManager, dialogSoundAndErrorStrings.openSound, 0, 0, 0, 0, kFloat_0_5_TextureDraw,
                         kFloat_200_805E0330, kFloat_1_PartyBalloon);
        p->openRequested = 1;
        p->closeRequested = 0;
        if (animate)
            DialogStartOpening(p);
        else
            DialogCompleteOpening(p);
    }
    return result;
}

struct DialogCloseOwnerPrefix {
    char pad[4];
};
class DialogCloseInterface : public DialogCloseOwnerPrefix {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual int v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
};
// Query the derived closing prerequisite (vtable +28); nonzero allows closing.
static inline int DialogAllowsClosing(InGameDialogView* dialog) {
    return ((DialogCloseInterface*)dialog)->v28();
}
// Dispatch the derived start-close hook (+38).
static inline void DialogStartClosing(InGameDialogView* dialog) {
    ((DialogCloseInterface*)dialog)->v38();
}
// Dispatch the derived complete-close hook (+20), bypassing animation.
static inline void DialogCompleteClosing(InGameDialogView* dialog) {
    ((DialogCloseInterface*)dialog)->v20();
}
// Request closing; return zero if already requested or vetoed by can-close.
// animate!=0 starts closing; zero completes closing immediately.
int CInGameDialog::RequestClose(InGameDialogView* p, int animate) {
    if (p->closeRequested != 0)
        return 0;
    int result = DialogAllowsClosing(p);
    if (result != 0) {
        Sound_PlayByName(g_soundManager, dialogSoundAndErrorStrings.closeSound, 0, 0, 0, 0, kFloat_0_5_TextureDraw,
                         kFloat_200_805E0330, kFloat_1_PartyBalloon);
        p->closeRequested = 1;
        p->openRequested = 0;
        if (animate)
            DialogStartClosing(p);
        else
            DialogCompleteClosing(p);
    }
    return result;
}

// Advance progress and auto-close using dt in the same units as duration/delay.
// No time-unit conversion occurs here. Return 0 for dt<=0 or not-ready,
// otherwise 1. Hooks may change state; the state is read after both hooks.
int CInGameDialog::UpdateTransition(InGameDialogView* p, float dt) {
    if (!(dt > kFloat_0_805E0328))
        return 0;
    ((void (*)(void*))p->vtable[kFrameHookSlot])(p);
    if (!((int (*)(void*))p->vtable[kReadySlot])(p))
        return 0;
    switch (p->dialogState) {
    case Dialog_Open:
        if (p->autoCloseDelay > kFloat_0_805E0328) {
            p->elapsedTime += dt;
            if (p->elapsedTime > p->autoCloseDelay)
                ((void (*)(void*, int))p->vtable[kRequestCloseSlot])(p, 1);
        }
        break;
    case Dialog_Opening:
        p->elapsedTime += dt;
        if (p->elapsedTime < p->transitionDuration)
            p->transitionProgress = p->elapsedTime / p->transitionDuration;
        else
            ((void (*)(void*))p->vtable[kCompleteOpenSlot])(p);
        break;
    case Dialog_Closing:
        p->elapsedTime += dt;
        if (p->elapsedTime < p->transitionDuration)
            p->transitionProgress = kFloat_1_PartyBalloon - p->elapsedTime / p->transitionDuration;
        else
            ((void (*)(void*))p->vtable[kCompleteCloseSlot])(p);
        break;
    }
    return 1;
}

// Initialize empty dialog state, create the text renderer, and invoke the initial setup hook.
// A named static member, not a real constructor: callers run it on an already-allocated base object.
int CInGameDialog::Init(InGameDialogView* p) {
    // Phase 1: establish sole ownership and constructor-only scalar defaults.
    p->refcount = 1;
    p->vtable = (void**)__vt__13CInGameDialog;
    p->x08 = kFloat_0_805E0328;
    p->x0c = kFloat_0_805E0328;
    p->x10 = kFloat_0_805E0328;
    p->x14 = kFloat_0_805E0328;
    p->openRequested = 0;
    p->closeRequested = 1;
    p->resetState34 = 0;
    p->blockRegistryClose = 0;
    p->messageDraw = 0;
    p->stringStorageWord = 0;
    p->stringLength = 0;
    p->stringData = 0;
    // Phase 2: create the owned message renderer, then dispatch setup only if it exists.
    ((CPangyaMsgDrawObject*)(&p->messageDraw))->Create();
    if (*(int*)&((CPangyaMsgDrawObject*)p->messageDraw)->messageDraw == 0)
        return (int)p;
    void** vt = p->vtable;
    ((void (*)(void*))vt[kDialogSetupSlot])(p);
    return (int)p;
}

// Reset transition timers and erase the current inline or heap-backed wide-string text.
int CInGameDialog::ResetDialog() {
    InGameDialogView* p = (InGameDialogView*)this;
    float zero = kFloat_0_805E0328;
    unsigned int tag = *(unsigned int*)&AsCInGameDialog(p)->stringStorageFlags;
    // Phase 1: choose inline or owned UTF-16 storage; the high bit selects heap storage.
    int dynamic = tag >> 31;
    p->dialogState = Dialog_Reset;
    p->autoCloseDelay = p->transitionProgress = p->elapsedTime = p->transitionDuration = zero;
    unsigned char* raw = (unsigned char*)p;
    char* base;
    int count;
    if (!dynamic) {
        base = (char*)p->shortString.inlineText;
        count = AsCInGameDialog(raw)->stringStorageFlags & 0x7f;
    } else {
        base = (char*)p->stringData;
        count = AsCInGameDialog(p)->stringLength;
    }
    char* end = base + count * 2;
    int flag = !dynamic;
    char* other;
    if (flag)
        other = (char*)p->shortString.inlineText;
    else
        other = (char*)p->stringData;
    // WString_ReplaceRange takes UTF-16 code-unit indices; pointer deltas above are bytes.
    int half0 = (int)(end - other);
    int index0 = half0 / 2;
    char* other2;
    if (flag)
        other2 = (char*)p->shortString.inlineText;
    else
        other2 = (char*)p->stringData;
    int half1 = (int)(other - other2);
    int index1 = half1 / 2;
    unsigned char ch = 0;
    WString_ReplaceRange(&p->stringStorageWord, index1, index0, 0, 0, &ch);
    return 1;
}

// Run close/reset hooks, release the text renderer and string, and optionally free the dialog.
int __dt__15SmallDialogBaseFv(InGameDialogView* p, int deleting) {
    if (p) {
        p->vtable = (void**)__vt__13CInGameDialog;
        ((void (*)(void*, int))p->vtable[kRequestCloseSlot])(p, 0);
        ((void (*)(void*))p->vtable[kCompleteCloseSlot])(p);
        if (p->messageDraw) {
            int n = --*(int*)p->messageDraw;
            if (!n)
                ((void (**)(void*, int)) * (void***)((char*)p->messageDraw + 4))[2](p->messageDraw, 1);
            p->messageDraw = 0;
        }
        if (((char*)&AsCInGameDialog(p)->stringStorageFlags)) {
            if (*(unsigned int*)&p->stringStorageWord >> 31)
                appFreeUni__FPv(p->stringData);
        }
        if (deleting > 0)
            __dl__FPv(p);
    }
    return (int)p;
}

struct CharVector802558EC {
    unsigned char* data;
    unsigned int size;
    unsigned int capacity;
};

void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* memmove(void*, const void*, unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);

// Select the larger insertion growth increment without discarding volatile reads.
inline const volatile unsigned int& max802558EC(const volatile unsigned int& a, const volatile unsigned int& b) {
    return a < b ? b : a;
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Insert one byte and return its new address (growth invalidates old iterators).
// pos is in [data,data+size]; val borrows a readable byte, including an
// existing element. flag is an unused allocator marker, not an output flag.
unsigned char* ByteVector_InsertOne(void* object, void* pos, void* val, unsigned char* flag) {
    CharVector802558EC* self;
    unsigned char* value;
    unsigned char* position;
    unsigned int capacity;
    self = (CharVector802558EC*)object;
    position = (unsigned char*)pos;
    value = (unsigned char*)val;
    (void)flag;
    unsigned int size = self->size;
    capacity = self->capacity;
    // Original 0x802558EC: preserve validation, in-place insertion, then allocation.
    do {
        if (size >= capacity) {
            if ((unsigned int)(-1 - capacity) < 1U) {
                THROW_LENGTH_ERROR_802558EC(1);
            }
            do {
                if (capacity < 0x55555555u)
                    break;
                else if (capacity < 0xAAAAAAAAu)
                    break;
                break;
            } while (0);
        } else {
            {
                unsigned char* end = self->data + size;
                if (position <= value && value < end)
                    value++;
                unsigned int tail = (unsigned int)(end - position);
                unsigned char* oldend = end + 1;
                ++self->size;
                memmove(oldend - tail, position, tail);
                *position = *value;
            }
            break;
        }
        {
            unsigned char* old = self->data;
            volatile unsigned int one = 1;
            unsigned int offset = (unsigned int)(position - old);
            capacity = self->capacity;
            if ((unsigned int)(-1 - capacity) < 1U) {
                THROW_LENGTH_ERROR_802558EC(2);
            }
            unsigned int newCapacity;
            unsigned int half;
            unsigned int grown;
            if (capacity < 0x55555555u) {
                unsigned int c = capacity + 1;
                grown = (c * 4 - c) / 5;
                newCapacity = capacity + max802558EC(grown, one);
            } else if (capacity < 0xAAAAAAAAu) {
                half = (capacity + 1) / 2;
                newCapacity = capacity + max802558EC(half, one);
            } else {
                newCapacity = 0xFFFFFFFFu;
            }
            unsigned char* fresh = (unsigned char*)appMallocDummy__FUl(newCapacity);
            self->data = fresh;
            self->capacity = newCapacity;
            fresh[offset] = *value;
            if (old != 0) {
                unsigned char* d = self->data;
                memmove(d, old, offset);
                unsigned char* end = old + self->size;
                unsigned char* from = old + offset;
                memmove(d + offset + 1, from, (unsigned int)(end - from));
                appFreeUni__FPv(old);
            }
            ++self->size;
            position = self->data + offset;
        }
    } while (0);
    return position;
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset

#undef THROW_VECTOR_LENGTH_ERROR_80254BD4
#undef THROW_LENGTH_ERROR_802558EC

struct DialogInactiveStateView {
    char padding00[0x18];
    int state;
    char padding1C[8];
    float value;
};

struct DialogOpenStateView {
    char padding00[0x18];
    int state;
    char padding1C[4];
    float value0;
    float value1;
};

// Initialize the unit-local full-turn constant.
void Dialog_InitializeFullTurnConstant() {
    s_dialogFullTurn = kFloat_6_2831855;
}

// Mark the dialog inactive and clear its transition progress.
int CInGameDialog::CompleteClose(DialogInactiveStateView* arg) {
    float z = kFloat_0_805E0328;
    arg->state = Dialog_Inactive;
    arg->value = z;
    return 1;
}

// Mark the dialog fully open with zero elapsed time.
void CInGameDialog::CompleteOpen(DialogOpenStateView* arg) {
    float o;
    float z;
    z = kFloat_0_805E0328;
    o = kFloat_1_PartyBalloon;
    arg->state = Dialog_Open;
    arg->value0 = z;
    arg->value1 = o;
}

// Insert one byte at the iterator read from the supplied pointer slot.
void ByteVector_InsertAtIterator(void* byteVector, void** arg1, void* arg2) {
    void* value = *(void* volatile*)arg1;
    unsigned char flag = 0;
    ByteVector_InsertOne(byteVector, value, arg2, &flag);
}

struct DialogCallbackTable {
    char padding00[0x20];
    void (*method20)(void*);
    char padding24[0x0C];
    void (*method30)(void*);
};

struct DialogTransitionStateView {
    char padding00[4];
    DialogCallbackTable* vtable;
    char padding08[0x10];
    int state;
    float timer;
    float value0;
    float value1;
};

// Start the opening transition or invoke the immediate-open callback.
void CInGameDialog::StartOpening() {
    DialogTransitionStateView* dialog = (DialogTransitionStateView*)this;
    if (dialog->timer > kFloat_0_805E0328) {
        float z = kFloat_0_805E0328;
        dialog->value0 = z;
        dialog->state = Dialog_Opening;
        dialog->value1 = z;
        return;
    }
    void* vt = AsCInGameDialog(dialog)->callbackVtable;
    void (*m30)(void*) = ((DialogCallbackTable*)vt)->method30;
    m30(dialog);
}

// Start the closing transition or invoke the immediate-close callback.
void CInGameDialog::StartClosing() {
    DialogTransitionStateView* dialog = (DialogTransitionStateView*)this;
    float t = dialog->timer;
    float z = kFloat_0_805E0328;
    if (t > z) {
        float o = kFloat_1_PartyBalloon;
        dialog->state = Dialog_Closing;
        dialog->value0 = z;
        dialog->value1 = o;
        return;
    }
    void* vt = AsCInGameDialog(dialog)->callbackVtable;
    void (*m20)(void*) = ((DialogCallbackTable*)vt)->method20;
    m20(dialog);
}

// Read the byte-vector data/iterator pointer.
void* ByteVector_GetDataPointer(void** arg) {
    return *arg;
}
// Return a byte offset into the vector storage.
void* ByteVector_GetOffsetPointer(void* byteVector, unsigned int offset) {
    return (char*)*(void**)byteVector + offset;
}
// Return the address of a byte-vector element.
void* ByteVector_GetElementPtr(void* byteVector, unsigned int offset) {
    return (char*)*(void**)byteVector + offset;
}
// Default open prerequisite: always allow.
int CInGameDialog::CanOpen() {
    return 1;
}
// Default close prerequisite: always allow.
int CInGameDialog::CanClose() {
    return 1;
}

// Default per-frame dialog hook: no action.
void CInGameDialog::OnFrame() {}
// Default dialog readiness predicate: always ready.
int CInGameDialog::IsReadyForUpdate() {
    return 1;
}
// Empty dialog extension hook; its specific derived role is unresolved.
void fn_80256260() {}
// Empty dialog extension hook; its specific derived role is unresolved.
void fn_8025629C() {}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622EC", _ctors_u80254BD4_0, Dialog_InitializeFullTurnConstant);
#pragma pop

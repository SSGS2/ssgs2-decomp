// Sound playback state: the two-slot groups that each track one playing sound, with a file
// handle and three owned strings per slot, and the playback table of hashed nodes that
// finds them. Covers construction, teardown and recycling of these objects.

#include "classes/SoundPlaybackState.h"
#include "classes/GtlSoundStreamInterface.h"
#include "classes/MWStringStorage.h"
#include "classes/CSharedFileHandle.h"

// Derive ABI displacements from the shared slot model while retaining byte-address
// access types: direct field replacements change alias/reload behavior in teardown.
#define PLAYBACK_SLOT_OFFSET(field) ((int)&((SoundPlaybackState*)0)->field)

extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Arm cleanup only after all three words of a string are initialized.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Transfer string-buffer ownership to the completed enclosing object.
    void release() {
        active = false;
    }
    // Free only heap-backed strings when construction unwinds.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u))
            appFreeUni__FPv((void*)words[2]);
    }
};

extern "C" {

extern const float kFloat_0_soundPlayback;
extern const float kFloat_1_SoundPlayback;
extern char s_soundPlaybackEmptyName[4];
extern char s_soundPlaybackEmptyFilePath[4];
extern char s_soundPlaybackEmptyResourcePath[4];

extern void FloatTween_Init(void*, float);
extern void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, unsigned char*);
extern void FloatTween_Start(void*, int, float, float);
extern void String_ReserveCapacity(void*, int);
void* SoundPlaybackSlot_Ctor(void*);
void* SoundPlaybackSlot_dtor(void*, int);
extern int strlen(const char*);

extern void appFreeUni__FPv(void*);
extern void __destroy_arr(void*, void*, int, int);
extern void __dl__FPv(void*);
extern void __construct_array(void*, void*, void*, int, int);
extern void OSInitMutex(void*);
extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern void* appMallocUni__FUl(int);
extern void String_ReserveCapacity(void*, int);

struct OneByte {
    char c;
};
enum PlaybackStorageConstants {
    // Offsets from the group-relative row (actual slot minus four).
    kRowStream = 0x04,
    kRowVolume = 0x08,
    kRowPitch = 0x0C,
    kRowPlayState = 0x10,
    kRowName = 0x14,
    kRowLoop = 0x20,
    kRowFadeIn = 0x21,
    kRowDolbyBank = 0x22,
    kRowTransitionDuration = 0x24,
    kRowTween = 0x28,
    kRowFilePath = 0x44,
    kRowResourcePath = 0x50,
    kRowBaseVolume = 0x60,
    kRowBasePitch = 0x64,
    kRowSharedFile = 0x5C,
    kNameReserveBytes = 0x40,
    kPathReserveBytes = 0x10,
    kPlaybackSlotBytes = 0x64,
    kPlaybackSlotsPerGroup = 2,
    kPlaybackInitialBucketCount = 16,
    kPlaybackBucketBytes = 16 * 4,
    kPlaybackGroupBOffset = 0xCC,
    kPlaybackTableAOffset = 0x198,
    kPlaybackTableBOffset = 0x1D0,
    kHeapStringTagShift = 31,
    kInlineStringLengthMask = 0x7F
};
struct RcPad4 {
    char pad[4];
};
struct RcObj4 : public RcPad4 {
    virtual void d0(int);
};
union Slot4 {
    unsigned char c;
    unsigned int pad;
};
struct PlaybackRingNode {
    char unk0[0xC];
    PlaybackRingNode* next;
    PlaybackRingNode* prev;
};
// Table offsets: +00 active ring, +04 iterator, +0C recycled ring,
// +10 initial bucket count, +14 bucket mask, +18 slot count, +1C owned
// bucket-pointer array, +20 recursive OS mutex. +08 is unresolved bookkeeping.
// Rings are circular with null meaning empty; links are embedded at +0C/+10
// in nodes. Clear splices rings and zeros buckets; it frees neither nodes nor
// their payloads. The two table instances live at owner +198 and +1D0.
struct PlaybackTableView {
    void* activeHead;   // Active circular ring head; null when empty.
    void* f04;          // Active iterator; reset when initializing the table.
    void* f08;          // Unresolved bookkeeping; initialized to null.
    void* recycledHead; // Recycled circular ring head; nodes remain allocated.
    int f10;            // Initial bucket count; subsequent resizing is outside this unit.
    int bucketMask;     // Bucket mask = allocated bucket count - 1.
    int bucketCount;    // Number of bucket pointers cleared by RecyclePlaybackNodes.
    void* buckets;      // Owned array of bucket pointers.
    char lock[0x18];    // Recursive OS mutex; nested operations require recursion.
};

// Read byte length from the Metrowerks inline/heap string tag.
// Keep this expression at the call site: an inline function changes .text here.
#define STR_LEN(q) ((*(unsigned int*)(q) >> 31) == 0 ? (*(unsigned char*)(q) & 0x7F) : *(unsigned int*)((char*)(q) + 4))

// Slot layout relative to the actual slot (0x64 bytes): stream/ref +00,
// volume/pitch +04/+08 (unit multipliers, default 1), state word +0C (idle=0),
// owned name/file/resource strings +10/+40/+4C,
// loop/fade-in/Dolby-bank flags +1C..1E (default false), transition duration +20,
// fade tween +24, shared-file handle +58,
// base-volume/base-pitch +5C/+60 (unit multipliers, default 1).
// Evidence: SoundPlaybackState.h and SoundStreamPlayback_Start in sound_manager.cpp.
// Shared kFloat_1_SoundPlayback is the same 1.0 storage; its suffix is misleading.
// row below points FOUR BYTES BEFORE the slot. This lets each two-slot group
// place a leading word before its array. sN/dN are allocator-tag scratch copies.
// This reset only clears the stream pointer; it does not stop/release a live
// stream or shared file. Callers must dispose those owners before resetting live slots.
// Keep the reset macro's expression lifetimes: the inline replacement changed .text.
#define SLOT_INIT(row, s1, d1, s2, d2, s3, d3, one, zero)                                                           \
    {                                                                                                               \
        /* A: detach stream and restore volume, pitch and idle state. */                                            \
        *(int*)((char*)(row) + kRowStream) = 0;                                                                     \
        *(float*)((char*)(row) + kRowVolume) = one;                                                                 \
        *(float*)((char*)(row) + kRowPitch) = one;                                                                  \
        *(int*)((char*)(row) + kRowPlayState) = 0;                                                                  \
        /* B: empty all owned strings using the original allocator-tag copies. */                                   \
        {                                                                                                           \
            char* q_ = (char*)(row) + kRowName;                                                                     \
            unsigned int n_ = STR_LEN(q_);                                                                          \
            (d1).c = (s1);                                                                                          \
            int l_ = strlen(s_soundPlaybackEmptyName);                                                              \
            String_ReplaceRange(q_, 0, n_, s_soundPlaybackEmptyName, s_soundPlaybackEmptyName + l_,                 \
                                (unsigned char*)&(d1));                                                             \
        }                                                                                                           \
        *(char*)((char*)(row) + kRowLoop) = 0;                                                                      \
        *(char*)((char*)(row) + kRowFadeIn) = 0;                                                                    \
        *(char*)((char*)(row) + kRowDolbyBank) = 0;                                                                 \
        *(float*)((char*)(row) + kRowTransitionDuration) = zero;                                                    \
        {                                                                                                           \
            char* q_ = (char*)(row) + kRowFilePath;                                                                 \
            unsigned int n_ = STR_LEN(q_);                                                                          \
            (d2).c = (s2);                                                                                          \
            int l_ = strlen(s_soundPlaybackEmptyFilePath);                                                          \
            String_ReplaceRange(q_, 0, n_, s_soundPlaybackEmptyFilePath, s_soundPlaybackEmptyFilePath + l_,         \
                                (unsigned char*)&(d2));                                                             \
        }                                                                                                           \
        {                                                                                                           \
            char* q_ = (char*)(row) + kRowResourcePath;                                                             \
            unsigned int n_ = STR_LEN(q_);                                                                          \
            (d3).c = (s3);                                                                                          \
            int l_ = strlen(s_soundPlaybackEmptyResourcePath);                                                      \
            String_ReplaceRange(q_, 0, n_, s_soundPlaybackEmptyResourcePath, s_soundPlaybackEmptyResourcePath + l_, \
                                (unsigned char*)&(d3));                                                             \
        }                                                                                                           \
        /* C: reset fade tween/base multipliers and retain string capacities. */                                    \
        {                                                                                                           \
            char* t_ = (char*)(row) + kRowTween;                                                                    \
            FloatTween_Start(t_, -1, kFloat_0_soundPlayback, kFloat_0_soundPlayback);                               \
        }                                                                                                           \
        *(float*)((char*)(row) + kRowBaseVolume) = one;                                                             \
        *(float*)((char*)(row) + kRowBasePitch) = one;                                                              \
        String_ReserveCapacity((char*)(row) + kRowName, kNameReserveBytes);                                         \
        String_ReserveCapacity((char*)(row) + kRowFilePath, kPathReserveBytes);                                     \
        String_ReserveCapacity((char*)(row) + kRowResourcePath, kPathReserveBytes);                                 \
    }

// Recycle active nodes and clear bucket pointers; caller already holds the recursive mutex.
static inline void RecyclePlaybackNodes(PlaybackTableView* c, void* mutex) {
    if ((c)->activeHead != 0 && (c)->recycledHead != 0) {
        ((PlaybackRingNode*)(c)->activeHead)->next->prev = ((PlaybackRingNode*)(c)->recycledHead)->prev;
        ((PlaybackRingNode*)(c)->recycledHead)->prev->next = ((PlaybackRingNode*)(c)->activeHead)->next;
        ((PlaybackRingNode*)(c)->recycledHead)->prev = (PlaybackRingNode*)(c)->activeHead;
        ((PlaybackRingNode*)(c)->activeHead)->next = (PlaybackRingNode*)(c)->recycledHead;
        (c)->activeHead = 0;
    } else {
        PlaybackRingNode* removed;
        while ((removed = (PlaybackRingNode*)(c)->activeHead) != 0) {
            OSLockMutex(mutex);
            removed->prev->next = removed->next;
            removed->next->prev = removed->prev;
            PlaybackRingNode* remainingHead = removed->prev;
            if (remainingHead == removed)
                remainingHead = 0;
            OSUnlockMutex(mutex);
            PlaybackRingNode* recycled = (PlaybackRingNode*)(c)->activeHead;
            PlaybackRingNode* recycledHead = (PlaybackRingNode*)(c)->recycledHead;
            OSLockMutex(mutex);
            if (recycledHead == 0) {
                recycled->next = recycled;
                recycled->prev = recycled;
                OSUnlockMutex(mutex);
            } else {
                recycled->prev = recycledHead;
                recycled->next = recycledHead->next;
                recycledHead->next = recycled;
                recycled->next->prev = recycled;
                OSUnlockMutex(mutex);
                recycled = recycledHead;
            }
            (c)->recycledHead = recycled;
            (c)->activeHead = remainingHead;
        }
    }
    for (int slot = 0; slot < (c)->bucketCount; ++slot)
        ((int*)(c)->buckets)[slot] = 0;
}

// Allocate sixteen empty buckets and initialize both circular node rings under the mutex.
static inline void InitializePlaybackTable(PlaybackTableView* c) {
    void* mutex = (c)->lock;
    (c)->buckets = 0;
    OSInitMutex(mutex);
    OSLockMutex(mutex);
    (c)->activeHead = 0;
    (c)->recycledHead = 0;
    (c)->f08 = 0;
    (c)->f10 = kPlaybackInitialBucketCount;
    (c)->f04 = 0;
    (c)->bucketCount = kPlaybackInitialBucketCount;
    if ((c)->bucketCount > 0) {
        (c)->buckets = appMallocUni__FUl(kPlaybackBucketBytes);
        (c)->bucketMask = (c)->bucketCount - 1;
        for (int slot = 0; slot < (c)->bucketCount; ++slot)
            ((int*)(c)->buckets)[slot] = 0;
    } else {
        (c)->buckets = 0;
    }
    OSUnlockMutex(mutex);
    OSLockMutex(mutex);
    RecyclePlaybackNodes(c, mutex);
    OSUnlockMutex(mutex);
}

// Lock and recycle the table's active nodes without freeing payloads or bucket storage.
static inline void ClearPlaybackTable(PlaybackTableView* c) {
    void* mutex = (c)->lock;
    OSLockMutex(mutex);
    RecyclePlaybackNodes(c, mutex);
    OSUnlockMutex(mutex);
}

// Forward initialization without altering the original inlining boundary.
static inline void InitializePlaybackTableAtOriginalBoundary(PlaybackTableView* c) {
    InitializePlaybackTable(c);
}
// Forward clearing without altering the original inlining boundary.
static inline void ClearPlaybackTableAtOriginalBoundary(PlaybackTableView* c) {
    ClearPlaybackTable(c);
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Construct two two-slot groups, initialize their tables, then repeat the original reset sequence.
void* SoundPlaybackState_Ctor(void* self) {
    char* stateBytes = (char*)self;
    // Stack-order group 1: first initialization of group A (name/file/resource tags).
    OneByte groupAInitialResourcePathTagCopy;
    volatile OneByte groupAInitialResourcePathTagSource;
    OneByte groupAInitialFilePathTagCopy;
    volatile OneByte groupAInitialFilePathTagSource;
    OneByte groupAInitialNameTagCopy;
    volatile OneByte groupAInitialNameTagSource;
    // Stack-order group 2: first initialization of group B.
    OneByte groupBInitialResourcePathTagCopy;
    volatile OneByte groupBInitialResourcePathTagSource;
    OneByte groupBInitialFilePathTagCopy;
    volatile OneByte groupBInitialFilePathTagSource;
    OneByte groupBInitialNameTagCopy;
    volatile OneByte groupBInitialNameTagSource;
    // Stack-order group 3: repeated reset of group B (used after the A reset below).
    OneByte groupBResetResourcePathTagCopy;
    volatile OneByte groupBResetResourcePathTagSource;
    OneByte groupBResetFilePathTagCopy;
    volatile OneByte groupBResetFilePathTagSource;
    OneByte groupBResetNameTagCopy;
    volatile OneByte groupBResetNameTagSource;
    // Stack-order group 4: repeated reset of group A.
    OneByte groupAResetResourcePathTagCopy;
    volatile OneByte groupAResetResourcePathTagSource;
    OneByte groupAResetFilePathTagCopy;
    volatile OneByte groupAResetFilePathTagSource;
    OneByte groupAResetNameTagCopy;
    volatile OneByte groupAResetNameTagSource;

    // Phase 1: construct group A at +004/+068 and group B at +0D0/+134.
    // Each group has a leading word (+000/+0CC) outside the 0x64-byte slots.
    __construct_array(stateBytes + 4, (void*)SoundPlaybackSlot_Ctor, (void*)SoundPlaybackSlot_dtor, kPlaybackSlotBytes,
                      kPlaybackSlotsPerGroup);
    *(int*)stateBytes = 0;
    {
        float one = kFloat_1_SoundPlayback;
        float zero = kFloat_0_soundPlayback;
        unsigned char groupAInitialNameTagByte = groupAInitialNameTagSource.c,
                      groupAInitialFilePathTagByte = groupAInitialFilePathTagSource.c,
                      groupAInitialResourcePathTagByte = groupAInitialResourcePathTagSource.c;
        unsigned int i = 0;
        int off = 0;
        for (; i < kPlaybackSlotsPerGroup; ++i, off += kPlaybackSlotBytes) {
            SLOT_INIT(stateBytes + off, groupAInitialNameTagByte, groupAInitialNameTagCopy,
                      groupAInitialFilePathTagByte, groupAInitialFilePathTagCopy, groupAInitialResourcePathTagByte,
                      groupAInitialResourcePathTagCopy, one, zero)
        }
    }
    char* groupBBytes = stateBytes + kPlaybackGroupBOffset;
    __construct_array(groupBBytes + 4, (void*)SoundPlaybackSlot_Ctor, (void*)SoundPlaybackSlot_dtor, kPlaybackSlotBytes,
                      kPlaybackSlotsPerGroup);
    *(int*)groupBBytes = 0;
    {
        float zero = kFloat_0_soundPlayback;
        float one = kFloat_1_SoundPlayback;
        unsigned char groupBInitialNameTagByte = groupBInitialNameTagSource.c,
                      groupBInitialFilePathTagByte = groupBInitialFilePathTagSource.c,
                      groupBInitialResourcePathTagByte = groupBInitialResourcePathTagSource.c;
        for (unsigned int i = 0; i < kPlaybackSlotsPerGroup; ++i) {
            SLOT_INIT(groupBBytes + i * kPlaybackSlotBytes, groupBInitialNameTagByte, groupBInitialNameTagCopy,
                      groupBInitialFilePathTagByte, groupBInitialFilePathTagCopy, groupBInitialResourcePathTagByte,
                      groupBInitialResourcePathTagCopy, one, zero)
        }
    }

    {
        // Phase 2: allocate empty bucket arrays; no active or recycled nodes remain.
        PlaybackTableView* c1 = (PlaybackTableView*)(stateBytes + kPlaybackTableAOffset);
        InitializePlaybackTableAtOriginalBoundary(c1);
    }
    {
        PlaybackTableView* c2 = (PlaybackTableView*)(stateBytes + kPlaybackTableBOffset);
        InitializePlaybackTableAtOriginalBoundary(c2);
    }

    // Phase 3: reset both constructed groups again, then clear both tables.
    // Repeated string replacements/reserves are intentional existing behavior.
    *(int*)stateBytes = 0;
    {
        float zero = kFloat_0_soundPlayback;
        float one = kFloat_1_SoundPlayback;
        unsigned char groupAResetNameTagByte = groupAResetNameTagSource.c,
                      groupAResetFilePathTagByte = groupAResetFilePathTagSource.c,
                      groupAResetResourcePathTagByte = groupAResetResourcePathTagSource.c;
        for (unsigned int i = 0; i < kPlaybackSlotsPerGroup; ++i) {
            SLOT_INIT(stateBytes + i * kPlaybackSlotBytes, groupAResetNameTagByte, groupAResetNameTagCopy,
                      groupAResetFilePathTagByte, groupAResetFilePathTagCopy, groupAResetResourcePathTagByte,
                      groupAResetResourcePathTagCopy, one, zero)
        }
    }
    *(int*)(stateBytes + kPlaybackGroupBOffset) = 0;
    {
        float one = kFloat_1_SoundPlayback;
        float zero = kFloat_0_soundPlayback;
        unsigned char groupBResetNameTagByte = groupBResetNameTagSource.c,
                      groupBResetFilePathTagByte = groupBResetFilePathTagSource.c,
                      groupBResetResourcePathTagByte = groupBResetResourcePathTagSource.c;
        for (unsigned int i = 0; i < kPlaybackSlotsPerGroup; ++i) {
            SLOT_INIT(stateBytes + kPlaybackGroupBOffset + i * kPlaybackSlotBytes, groupBResetNameTagByte,
                      groupBResetNameTagCopy, groupBResetFilePathTagByte, groupBResetFilePathTagCopy,
                      groupBResetResourcePathTagByte, groupBResetResourcePathTagCopy, one, zero)
        }
    }

    {
        PlaybackTableView* c3 = (PlaybackTableView*)(stateBytes + kPlaybackTableAOffset);
        ClearPlaybackTableAtOriginalBoundary(c3);
    }
    {
        PlaybackTableView* c4 = (PlaybackTableView*)(stateBytes + kPlaybackTableBOffset);
        ClearPlaybackTableAtOriginalBoundary(c4);
    }
    return self;
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Construct one slot, retaining three owned string buffers with unwind guards and an idle tween.
void* SoundPlaybackSlot_Ctor(void* self) {
    char* p = (char*)self;
    Slot4 d3;
    volatile Slot4 s3;
    Slot4 d2;
    volatile Slot4 s2;
    Slot4 d1;
    volatile Slot4 s1;
    int stringLength = 0;
    *(int*)(p + 0x10) = 0;
    *(int*)(p + 0x14) = 0;
    *(int*)(p + 0x18) = 0;
    EH5StringCleanup eh5Cleanup23(p + 0x10);
    FloatTween_Init(p + 0x24, kFloat_0_soundPlayback);
    *(int*)(p + 0x40) = 0;
    *(int*)(p + 0x44) = 0;
    *(int*)(p + 0x48) = 0;
    EH5StringCleanup eh5Cleanup24(p + 0x40);
    *(int*)(p + 0x4C) = 0;
    *(int*)(p + 0x50) = 0;
    *(int*)(p + 0x54) = 0;
    EH5StringCleanup eh5Cleanup25(p + 0x4C);
    *(int*)(p + 0x58) = 0;
    *(int*)p = 0;
    *(float*)(p + 0x04) = kFloat_1_SoundPlayback;
    *(float*)(p + 0x08) = kFloat_1_SoundPlayback;
    *(int*)(p + 0x0C) = 0;
    if ((*(unsigned int*)(p + 0x10) >> 31) == 0) {
        stringLength = *(unsigned char*)(p + 0x10) & 0x7F;
    } else {
        stringLength = *(int*)(p + 0x14);
    }
    d1.c = s1.c;
    int len1 = strlen(s_soundPlaybackEmptyName);
    String_ReplaceRange(p + 0x10, 0, (unsigned int)stringLength, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + len1, &d1.c);

    *(unsigned char*)(p + 0x1C) = 0;
    *(unsigned char*)(p + 0x1D) = 0;
    *(unsigned char*)(p + 0x1E) = 0;
    *(float*)(p + 0x20) = kFloat_0_soundPlayback;
    if ((*(unsigned int*)(p + 0x40) >> 31) == 0) {
        stringLength = *(unsigned char*)(p + 0x40) & 0x7F;
    } else {
        stringLength = *(int*)(p + 0x44);
    }
    d2.c = s2.c;
    int len2 = strlen(s_soundPlaybackEmptyFilePath);
    String_ReplaceRange(p + 0x40, 0, (unsigned int)stringLength, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + len2, &d2.c);

    if ((*(unsigned int*)(p + 0x4C) >> 31) == 0) {
        stringLength = *(unsigned char*)(p + 0x4C) & 0x7F;
    } else {
        stringLength = *(int*)(p + 0x50);
    }
    d3.c = s3.c;
    int len3 = strlen(s_soundPlaybackEmptyResourcePath);
    String_ReplaceRange(p + 0x4C, 0, (unsigned int)stringLength, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + len3, &d3.c);

    FloatTween_Start(p + 0x24, -1, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    *(float*)(p + 0x5C) = kFloat_1_SoundPlayback;
    *(float*)(p + 0x60) = kFloat_1_SoundPlayback;
    String_ReserveCapacity(p + 0x10, 0x40);
    String_ReserveCapacity(p + 0x40, 0x10);
    String_ReserveCapacity(p + 0x4C, 0x10);
    eh5Cleanup23.release();
    eh5Cleanup24.release();
    eh5Cleanup25.release();
    return self;
}

// Stop/drop the stream reference, delete the file handle, reset/free strings, and optionally free self.
void* SoundPlaybackSlot_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        char* slotBytes = (char*)self;
        volatile Slot4 nameAllocatorSource;
        Slot4 nameAllocatorCopy;
        volatile Slot4 filePathAllocatorSource;
        Slot4 filePathAllocatorCopy;
        volatile Slot4 resourcePathAllocatorSource;
        Slot4 resourcePathAllocatorCopy;
        // Phase 1: release external playback/file ownership before resetting slot data.
        void* stream = *(void**)slotBytes;
        if (stream != 0) {
            ((gtl::ISoundStream*)stream)->Stop(0);
            void* retainedStream = *(void**)slotBytes;
            int referenceCount = *(int*)retainedStream;
            referenceCount = referenceCount - 1;
            *(int*)retainedStream = referenceCount;
            if (referenceCount == 0 && retainedStream != 0) {
                ((RcObj4*)retainedStream)->d0(1);
            }
        }
        void* sharedFile = *(void**)(slotBytes + PLAYBACK_SLOT_OFFSET(sharedFile));
        if (sharedFile != 0) {
            CSharedFileHandle::Destroy(sharedFile, 1);
            *(void**)(slotBytes + PLAYBACK_SLOT_OFFSET(sharedFile)) = 0;
        }
        // Phase 2: restore defaults and empty strings, preserving the reserve calls
        // that precede destruction even though the strings are about to be freed.
        unsigned int nameTag = *(unsigned int*)(slotBytes + PLAYBACK_SLOT_OFFSET(nameFlags));
        *(void**)slotBytes = 0;
        *(float*)(slotBytes + PLAYBACK_SLOT_OFFSET(volume)) = kFloat_1_SoundPlayback;
        *(float*)(slotBytes + PLAYBACK_SLOT_OFFSET(pitch)) = kFloat_1_SoundPlayback;
        *(int*)(slotBytes + PLAYBACK_SLOT_OFFSET(playState)) = 0;
        int stringLength;
        if ((nameTag >> 31) == 0) {
            stringLength = *(unsigned char*)(slotBytes + PLAYBACK_SLOT_OFFSET(nameFlags)) & 0x7F;
        } else {
            stringLength = *(int*)(slotBytes + PLAYBACK_SLOT_OFFSET(nameLength));
        }
        nameAllocatorCopy.c = nameAllocatorSource.c;
        int emptyNameLength = strlen(s_soundPlaybackEmptyName);
        String_ReplaceRange(slotBytes + PLAYBACK_SLOT_OFFSET(nameFlags), 0, (unsigned int)stringLength,
                            s_soundPlaybackEmptyName, s_soundPlaybackEmptyName + emptyNameLength, &nameAllocatorCopy.c);

        unsigned int filePathTag = *(unsigned int*)(slotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags));
        *(unsigned char*)(slotBytes + PLAYBACK_SLOT_OFFSET(loopEnabled)) = 0;
        *(unsigned char*)(slotBytes + PLAYBACK_SLOT_OFFSET(fadeInEnabled)) = 0;
        *(unsigned char*)(slotBytes + PLAYBACK_SLOT_OFFSET(useDolbyBank)) = 0;
        *(float*)(slotBytes + PLAYBACK_SLOT_OFFSET(transitionDuration)) = kFloat_0_soundPlayback;
        if ((filePathTag >> 31) == 0) {
            stringLength = *(unsigned char*)(slotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags)) & 0x7F;
        } else {
            stringLength = *(int*)(slotBytes + PLAYBACK_SLOT_OFFSET(filePathLength));
        }
        filePathAllocatorCopy.c = filePathAllocatorSource.c;
        int emptyFilePathLength = strlen(s_soundPlaybackEmptyFilePath);
        String_ReplaceRange(slotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags), 0, (unsigned int)stringLength,
                            s_soundPlaybackEmptyFilePath, s_soundPlaybackEmptyFilePath + emptyFilePathLength,
                            &filePathAllocatorCopy.c);

        unsigned int resourcePathTag = *(unsigned int*)(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags));
        if ((resourcePathTag >> 31) == 0) {
            stringLength = *(unsigned char*)(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags)) & 0x7F;
        } else {
            stringLength = *(int*)(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathLength));
        }
        resourcePathAllocatorCopy.c = resourcePathAllocatorSource.c;
        int emptyResourcePathLength = strlen(s_soundPlaybackEmptyResourcePath);
        String_ReplaceRange(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags), 0, (unsigned int)stringLength,
                            s_soundPlaybackEmptyResourcePath,
                            s_soundPlaybackEmptyResourcePath + emptyResourcePathLength, &resourcePathAllocatorCopy.c);

        FloatTween_Start(slotBytes + PLAYBACK_SLOT_OFFSET(fadeMode), -1, kFloat_0_soundPlayback,
                         kFloat_0_soundPlayback);
        *(float*)(slotBytes + PLAYBACK_SLOT_OFFSET(baseVolume)) = kFloat_1_SoundPlayback;
        *(float*)(slotBytes + PLAYBACK_SLOT_OFFSET(basePitch)) = kFloat_1_SoundPlayback;
        String_ReserveCapacity(slotBytes + PLAYBACK_SLOT_OFFSET(nameFlags), 0x40);
        String_ReserveCapacity(slotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags), 0x10);
        String_ReserveCapacity(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags), 0x10);

        // Phase 3: free heap-backed strings in reverse member order; inline bytes need no free.
        if (slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags)) {
            unsigned int first = *(unsigned int*)(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags));
            if (first >> 31) {
                void* ptr = *(void**)(slotBytes + PLAYBACK_SLOT_OFFSET(resourcePathData));
                appFreeUni__FPv(ptr);
            }
        }
        if (slotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags)) {
            unsigned int first = *(unsigned int*)(slotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags));
            if (first >> 31) {
                void* ptr = *(void**)(slotBytes + PLAYBACK_SLOT_OFFSET(filePathData));
                appFreeUni__FPv(ptr);
            }
        }
        if (slotBytes + PLAYBACK_SLOT_OFFSET(nameFlags)) {
            unsigned int first = *(unsigned int*)(slotBytes + PLAYBACK_SLOT_OFFSET(nameFlags));
            if (first >> 31) {
                void* ptr = *(void**)(slotBytes + PLAYBACK_SLOT_OFFSET(nameData));
                appFreeUni__FPv(ptr);
            }
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy one two-slot group (leading word plus slots), optionally deleting that group storage.
void* SoundPlaybackGroup_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        char* groupBytes = (char*)self;
        unsigned char allocatorTagBytes[0x30];
        float defaultMultiplierThenZero = kFloat_1_SoundPlayback;
        char* releaseRowOrigin = groupBytes;
        unsigned char releaseNameAllocatorByte = allocatorTagBytes[0];
        char* releaseSlotBytes = groupBytes + 4;
        float zeroThenDefaultMultiplier = kFloat_0_soundPlayback;
        unsigned int releaseSlotIndex = 0;
        unsigned char releaseFileAllocatorByte = allocatorTagBytes[8];
        int zero = 0;
        unsigned char releaseResourceAllocatorByte = allocatorTagBytes[0x10];
        // Phase 1: stop/release each stream and file, then reset the still-live strings.
        for (releaseSlotIndex = 0; releaseSlotIndex < kPlaybackSlotsPerGroup; ++releaseSlotIndex) {
            void* stream = *(void**)(releaseRowOrigin + kRowStream);
            if (stream != 0) {
                ((gtl::ISoundStream*)stream)->Stop(0);
                int* streamReferenceCount = *(int**)(releaseRowOrigin + kRowStream);
                if (--*streamReferenceCount == 0) {
                    if (streamReferenceCount != 0) {
                        void** vt = *(void***)((char*)streamReferenceCount + 4);
                        ((void (*)(void*, int))vt[2])(streamReferenceCount, 1);
                    }
                }
            }
            void* sharedFile = *(void**)(releaseRowOrigin + kRowSharedFile);
            if (sharedFile != 0) {
                CSharedFileHandle::Destroy(sharedFile, 1);
                *(void**)(releaseRowOrigin + kRowSharedFile) = 0;
            }
            *(void**)(releaseRowOrigin + kRowStream) = 0;
            *(float*)(releaseRowOrigin + kRowVolume) = defaultMultiplierThenZero;
            *(float*)(releaseRowOrigin + kRowPitch) = defaultMultiplierThenZero;
            *(int*)(releaseRowOrigin + kRowPlayState) = 0;
            unsigned int nameLength =
                ((*(unsigned int*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags)) >> 31) == 0)
                    ? (*(unsigned char*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags)) & 0x7F)
                    : *(unsigned int*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(nameLength));
            allocatorTagBytes[4] = releaseNameAllocatorByte;
            int emptyStringLength = strlen(s_soundPlaybackEmptyName);
            String_ReplaceRange(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags), 0, nameLength,
                                s_soundPlaybackEmptyName, s_soundPlaybackEmptyName + emptyStringLength,
                                &allocatorTagBytes[4]);
            *(char*)(releaseRowOrigin + kRowLoop) = zero;
            *(char*)(releaseRowOrigin + kRowFadeIn) = zero;
            *(char*)(releaseRowOrigin + kRowDolbyBank) = zero;
            *(float*)(releaseRowOrigin + kRowTransitionDuration) = zeroThenDefaultMultiplier;
            unsigned int filePathLength =
                ((*(unsigned int*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags)) >> 31) == 0)
                    ? (*(unsigned char*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags)) & 0x7F)
                    : *(unsigned int*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(filePathLength));
            allocatorTagBytes[12] = releaseFileAllocatorByte;
            emptyStringLength = strlen(s_soundPlaybackEmptyFilePath);
            String_ReplaceRange(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags), 0, filePathLength,
                                s_soundPlaybackEmptyFilePath, s_soundPlaybackEmptyFilePath + emptyStringLength,
                                &allocatorTagBytes[12]);
            unsigned int resourcePathLength =
                ((*(unsigned int*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags)) >> 31) == 0)
                    ? (*(unsigned char*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags)) & 0x7F)
                    : *(unsigned int*)(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathLength));
            allocatorTagBytes[20] = releaseResourceAllocatorByte;
            emptyStringLength = strlen(s_soundPlaybackEmptyResourcePath);
            String_ReplaceRange(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags), 0, resourcePathLength,
                                s_soundPlaybackEmptyResourcePath, s_soundPlaybackEmptyResourcePath + emptyStringLength,
                                &allocatorTagBytes[20]);
            FloatTween_Start(releaseRowOrigin + 0x28, -1, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
            *(float*)(releaseRowOrigin + kRowBaseVolume) = defaultMultiplierThenZero;
            *(float*)(releaseRowOrigin + kRowBasePitch) = defaultMultiplierThenZero;
            String_ReserveCapacity(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags), 0x40);
            String_ReserveCapacity(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags), 0x10);
            String_ReserveCapacity(releaseSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags), 0x10);
            releaseRowOrigin += kPlaybackSlotBytes;
            releaseSlotBytes += kPlaybackSlotBytes;
        }
        // Phase 2: clear the leading word and repeat default initialization for both slots.
        int zero2 = 0;
        // The two float temporaries exchange roles for this second reset pass.
        zeroThenDefaultMultiplier = kFloat_1_SoundPlayback;
        *(int*)groupBytes = zero2;
        defaultMultiplierThenZero = kFloat_0_soundPlayback;
        char* resetRowOrigin = groupBytes;
        unsigned char resetNameAllocatorByte = allocatorTagBytes[0x18];
        char* resetSlotBytes = groupBytes + 4;
        unsigned int resetSlotIndex = 0;
        unsigned char resetFileAllocatorByte = allocatorTagBytes[0x20];
        unsigned char resetResourceAllocatorByte = allocatorTagBytes[0x28];
        for (resetSlotIndex = 0; resetSlotIndex < kPlaybackSlotsPerGroup; ++resetSlotIndex) {
            *(int*)(resetRowOrigin + kRowStream) = zero2;
            *(float*)(resetRowOrigin + kRowVolume) = zeroThenDefaultMultiplier;
            *(float*)(resetRowOrigin + kRowPitch) = zeroThenDefaultMultiplier;
            *(int*)(resetRowOrigin + kRowPlayState) = 0;
            unsigned int resetNameLength =
                ((*(unsigned int*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags)) >> 31) == 0)
                    ? (*(unsigned char*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags)) & 0x7F)
                    : *(unsigned int*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(nameLength));
            allocatorTagBytes[0x1C] = resetNameAllocatorByte;
            int resetEmptyStringLength = strlen(s_soundPlaybackEmptyName);
            String_ReplaceRange(resetSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags), 0, resetNameLength,
                                s_soundPlaybackEmptyName, s_soundPlaybackEmptyName + resetEmptyStringLength,
                                &allocatorTagBytes[0x1C]);
            *(char*)(resetRowOrigin + kRowLoop) = zero2;
            *(char*)(resetRowOrigin + kRowFadeIn) = zero2;
            *(char*)(resetRowOrigin + kRowDolbyBank) = zero2;
            *(float*)(resetRowOrigin + kRowTransitionDuration) = defaultMultiplierThenZero;
            unsigned int resetFilePathLength =
                ((*(unsigned int*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags)) >> 31) == 0)
                    ? (*(unsigned char*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags)) & 0x7F)
                    : *(unsigned int*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(filePathLength));
            allocatorTagBytes[0x24] = resetFileAllocatorByte;
            resetEmptyStringLength = strlen(s_soundPlaybackEmptyFilePath);
            String_ReplaceRange(resetSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags), 0, resetFilePathLength,
                                s_soundPlaybackEmptyFilePath, s_soundPlaybackEmptyFilePath + resetEmptyStringLength,
                                &allocatorTagBytes[0x24]);
            unsigned int resetResourcePathLength =
                ((*(unsigned int*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags)) >> 31) == 0)
                    ? (*(unsigned char*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags)) & 0x7F)
                    : *(unsigned int*)(resetSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathLength));
            allocatorTagBytes[0x2C] = resetResourceAllocatorByte;
            resetEmptyStringLength = strlen(s_soundPlaybackEmptyResourcePath);
            String_ReplaceRange(resetSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags), 0, resetResourcePathLength,
                                s_soundPlaybackEmptyResourcePath,
                                s_soundPlaybackEmptyResourcePath + resetEmptyStringLength, &allocatorTagBytes[0x2C]);
            FloatTween_Start(resetRowOrigin + 0x28, -1, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
            *(float*)(resetRowOrigin + kRowBaseVolume) = zeroThenDefaultMultiplier;
            *(float*)(resetRowOrigin + kRowBasePitch) = zeroThenDefaultMultiplier;
            String_ReserveCapacity(resetSlotBytes + PLAYBACK_SLOT_OFFSET(nameFlags), 0x40);
            String_ReserveCapacity(resetSlotBytes + PLAYBACK_SLOT_OFFSET(filePathFlags), 0x10);
            String_ReserveCapacity(resetSlotBytes + PLAYBACK_SLOT_OFFSET(resourcePathFlags), 0x10);
            resetRowOrigin += kPlaybackSlotBytes;
            resetSlotBytes += kPlaybackSlotBytes;
        }
        // Phase 3: run both element destructors; those release the string allocations.
        __destroy_arr(groupBytes + 4, (void*)SoundPlaybackSlot_dtor, kPlaybackSlotBytes, kPlaybackSlotsPerGroup);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

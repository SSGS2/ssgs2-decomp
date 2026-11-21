#include <decomp/ct_member.h>
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#include "classes/GolfDocSnapshot8013.h"
// Original unit 0x802E486C-0x802E6030: CSharedDoc +0xB90 points to CGolfDoc;
// +0x808/+0x8C0/+0xAD8/+0xAF0 gate selection and +0xA04 is itemInventory.
// CGolfDoc +0x2B9 counts players; +0x3B6/+0x3C1 select hole/player records.
#include "classes/CVoiceManager.h"
#define AsCVoiceManager(p) ((CVoiceManager*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char voiceManagerStrings[];
}
}

// Translation unit 0x802E486C-0x802E6030 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// The runtime throw entry does not return to its caller.
extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));
#include "sdk_protos.h"
#include "game/exception.h"
#include <decomp/cw_bitfields.h>

extern "C" {

typedef unsigned int u32;
// Opaque filename-selector ABI words: inventory results are forwarded, never dereferenced.
typedef void* VoiceFilenameIndexWord;


extern void* g_sharedDoc;
extern void* ItemInventory_FindCharacterIndex(void*, void*);
extern void* ItemInventory_FindCaddieIndex(void*, void*);
extern void CVoiceManager_AppendFormattedVoiceName(void*, void*, int, void*, void*);
extern void* InstanceTable_Get();
extern void* GolfBall__Fv();
extern void* PreData__9CGolfBallFi(void*, int);
extern void* CQuadTree_GetInstance();
extern int GetGroundType__9CQuadTreeF7WVector(void*, void*);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned int);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float WVectorLen__FRC7WVector(void*);
extern void appFreeUni__FPv(void*);
extern void* String_ReserveCapacity(void*, unsigned int);
extern void* String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
extern void* __nwa__FUl(u32);
extern void* appMallocDummy__FUl(u32);
extern void* __nw__FUl(u32);
extern void* __dla__FPv(void*);

extern char voiceManagerStrings[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern void __dt__Q23std12length_errorFv();

/* std::string (12 bytes, small-string optimisation). */
struct Alloc { char c __attribute__((aligned(4))); };

struct Str {
    union {
        unsigned int flag;
        struct { CW_BITS2( unsigned int heapbit : 1, unsigned int capacity : 31) };
        struct { CW_BITS2( unsigned char byteheap : 1, unsigned char smallsize : 7) };
    };
    unsigned int size;
    char* ptr;
    // Initialize an empty inline voice-name string.
    Str() { flag = 0; size = 0; ptr = 0; }
};

// Candidate vector elements are 12-byte owned strings. PendingVoiceVector has
// its own header type for 16-byte records; never index that storage as Str elements.
struct OutVec {
    Str* begin;
    u32 size;
    u32 cap;
};

// Pending payload: copied voice filename plus delay in seconds. CVoiceManager_OnProcess
// (stroke_point_2d_selector.cpp) subtracts elapsedSeconds from +0xC and plays when it becomes negative.
struct Entry {
    Str name;
    float delaySeconds; // delay seconds, consumed by CVoiceManager_OnProcess
};

// Pending storage has the same header shape as OutVec but a DIFFERENT stride.
// Every element owns its name; append deep-copies before publishing size.
struct PendingVoiceVector { Entry* begin; u32 size; u32 cap; };
typedef char VoiceCandidateString_size[(sizeof(Str) == 12) ? 1 : -1];
typedef char PendingVoiceEntry_size[(sizeof(Entry) == 16) ? 1 : -1];

struct Vec3 {
    float x, y, z;
};

// ClubSelectionStorage/ClubParameterView in club_selection_and_shot_params.cpp establish these fields.
// InstanceTable_Get returns that club-selection singleton, not a voice-manager object.
struct VoiceClubParameters {
    float distanceYards, launchAngle;
    unsigned char parameterKind, clubIndex, clubGroup;
};
struct VoiceClubSelection {
    void* vtable;
    VoiceClubParameters* selected;
    char registryAndSelection[0x3C];
    int powerMode;
};
typedef char VoiceClubSelectionPowerOffset[((int)&((VoiceClubSelection*)0)->powerMode == 0x44) ? 1 : -1];
enum { kNormalPowerMode = 0, kPowerShotMode = 1, kDoublePowerShotMode = 2,
       kLastLongClubVoiceIndex = 7, kClubGroupWood = 0, kClubGroupIron = 1 };

// Keep address expressions at their use sites: extracting these into even forced-inline
// accessors changes repeated-load ordering under this unit's CodeWarrior pragmas.
#define VOICE_DOCUMENT_BYTES ((char*)g_sharedDoc)
#define VOICE_GOLF_DOCUMENT_BYTES ((char*)AsCSharedDoc(g_sharedDoc)->golfDoc)
// Borrowed hole record: position is in world coordinates; par is a stroke count.
struct VoiceHoleRecord { char unknown00[0xC]; Vec3 position; unsigned char par; char unknown19[7]; };
// Equipment is embedded in the shared document, not the golf-player snapshot.
struct VoiceEquipmentSlot { char prefix[0x120]; void* character; char unknown124[0x94]; void* caddie; };
enum VoiceStorageContract {
    kVoiceHoleRecordBytes = 0x20, kVoiceEquipmentStride = 0x1D4,
    kVoiceResultShift = 4, kVoiceResultMask = 0xF,
    kVoiceStrokeRule = 0, kVoiceOpponentRule = 1,
    kVoiceResultMode0 = 0, kVoiceResultMode6 = 6,
    kPairedOpponentSlotBit = 1 // adjacent slots 0/1 and 2/3 are opponents
};
typedef char VoiceHoleRecord_size[(sizeof(VoiceHoleRecord) == kVoiceHoleRecordBytes) ? 1 : -1];
// Current hole's stroke byte and a player's high result nibble (values tested below).
#define CURRENT_VOICE_HOLE_STROKES (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer))->holeStrokeCounts[((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex])
#define VOICE_PLAYER_RESULT_NIBBLE(i) ((((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, (i)))->rankFlags >> kVoiceResultShift) & kVoiceResultMask)

// Event codes are supplied by the gameplay message word at +4. Unknown labels
// deliberately preserve unresolved semantics rather than inventing a voice meaning.
enum VoiceMessageEvent {
    kVoiceOpeningEvent = 0, kVoiceNormalClubEvent = 1, kVoicePoweredClubEvent = 2,
    kVoiceUnresolvedEvent3 = 3, kVoiceStrokeDistanceEvent = 4, kVoiceLandingEvent = 5,
    kVoiceHoleResultEvent = 6, kVoiceRoundResultEvent = 7, kVoiceUnresolvedEvent8 = 8,
    kVoiceResultQualifierEvent = 9, kVoiceUnresolvedEvent10 = 10
};
// Filename category IDs are fixed asset ABI. Labels describe proven selection
// conditions, not transcriptions of speech: the sound assets have not been auditioned.
// Primary/Alternate means separate candidates added by the same event, not speaker.
// Result mapping: strokes-par 0 -> 3; -1 birdie -> 4 (+16 in stroke play);
// -2 eagle -> 5 (+16); one stroke or <= -3 -> 6; over par -> 7 (+17).
// Opponent rule first requires fewer strokes than the paired opponent (slot ^ 1);
// >= par can then also use 3. A loss uses 7 and a tie adds nothing.
// Landing 23/24 are stronger/moderate response branches; 25 tests ball byte +0x58
// or terrain 3/12/10. No terrain/spoken interpretation has been established for 25.
// Opening uses 1+14; first stroke uses 2+15; each collected candidate is equally likely.
enum VoiceRelativeScore { kVoiceThreeUnderPar = -3, kVoiceTwoUnderPar = -2, kVoiceOneUnderPar = -1, kVoiceAtPar = 0 };

enum VoiceCategory {
    kVoiceOpeningPrimary = 1,
    kVoiceFirstStrokePrimary = 2,
    kVoiceParOrOtherHoleWin = 3,
    kVoiceBirdie = 4,
    kVoiceEagle = 5,
    kVoiceSingleStrokeOrAtLeastThreeUnderPar = 6,
    kVoiceOverParOrHoleLoss = 7,
    kVoiceQualifiedRoundSuccess = 8,
    kVoiceQualifiedRoundFailure = 9,
    kVoiceLongClub = 10,
    kVoiceShortClubOrOtherDoublePower = 11,
    kVoicePowerShotOrDoublePowerIron = 12,
    kVoiceDoublePowerWood = 13,
    kVoiceOpeningAlternate = 14,
    kVoiceFirstStrokeAlternate = 15,
    kVoiceBirdieOrEagleAlternate = 16,
    kVoiceOverParAlternate = 17,
    kVoiceRoundSuccessAlternate = 18,
    kVoiceRoundFailureAlternate = 19,
    kVoiceDistanceOver200Yards = 20,
    kVoiceDistanceOver50Yards = 21,
    kVoiceUnresolvedEvent3Category = 22,
    kVoiceStrongLandingResponse = 23,
    kVoiceModerateLandingResponse = 24,
    kVoiceLandingSpecialCase25 = 25,
    kVoiceUnresolvedEvent8Category = 26,
};
static const unsigned int kMaxPendingVoiceEntries = 0x0FFFFFFFu; // 16-byte entries
static const unsigned int kPendingVoiceSmallGrowthLimit = 0x05555555u;
static const unsigned int kPendingVoiceHalfGrowthLimit = 0x0AAAAAAAu;

/* Grow guard used by the vector's push_back slow path. */
struct GrowEntry {
    Entry* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int position;
    // Start an empty guard; [position, position+size) tracks constructed entries.
    GrowEntry(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), position(0) {}
    // Destroy constructed strings and release storage unless exchanged with the owner.
    ~GrowEntry() {
        Entry* start = data + position;
        Entry* end = start + size;
        volatile int destroyAlloc;
        *(volatile char*)&destroyAlloc = 0;
        while (end > start) {
            --end;
            if (end != 0) {
                if (end != 0 && (end->name.flag >> 31) != 0) appFreeUni__FPv(end->name.ptr);
            }
        }
        size = 0;
        if (data != 0) {
            Entry* base = data;
            int count = (int)size;
            {
                volatile int eraseAlloc;
                *(volatile char*)&eraseAlloc = 0;
                base += count;
                size -= count;
                while (count != 0) {
                    --base;
                    if (base != 0) {
                        if (base != 0 && (base->name.flag >> 31) != 0) appFreeUni__FPv(base->name.ptr);
                    }
                    count = count - 1;
                }
                appFreeUni__FPv(data);
            }
        }
    }
};

// Return the larger requested growth increment by reference.
inline const unsigned int& growMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

struct ErrorCount {
    void* vtable;
    int a, b;
    void* text;
    // Own copied error text through the standard shared-character deleter.
    ErrorCount(void* p) { a = 1; b = 1; vtable = s_sharedCharDeleterVtable; text = p; }
};

struct ErrorGuard_u802E486C {
    void* ptr;
    void* alloc;
    // Guard a newly allocated error-message array until its owner is built.
    ErrorGuard_u802E486C(void* p, void* a) : ptr(p), alloc(a) {}
    // Release the guarded error text if ownership did not transfer.
    ~ErrorGuard_u802E486C() { if (ptr) __dla__FPv(ptr); }
};

struct ErrorRoot : std::exception {
    // Write the exception vtable without generating a different class layout.
    void SetTable(const void* vt) { *(const void**)this = vt; }

    // Install the logic-error base vtable and preserve the allocator token write.
    ErrorRoot() { SetTable(__vt__Q23std11logic_error);
        volatile int alloc;
        *(volatile char*)&alloc = 0;
    }
};

struct ErrorBase_u802E486C {
    void* vtable;
    char* text;
    ErrorCount* count;
    // Allocate/copy the error message and transfer it to a shared owner.
    ErrorBase_u802E486C(const char* const& msg) : vtable(__vt__Q23std11logic_error) {
        volatile int rootAlloc;
        *(volatile char*)&rootAlloc = 0;
        volatile int alloc;
        *(volatile char*)&alloc = 0;

        unsigned int len = strlen(msg) + 1;
        char* saved = (char*)__nwa__FUl(len * 2 - len);
        text = saved;
        {
            ErrorGuard_u802E486C guard(saved, (void*)&alloc);
            count = new ErrorCount(saved);
            guard.ptr = 0;
        }
        strcpy(text, msg);
    }
    // Delegate exception-base destruction to the runtime.
    ~ErrorBase_u802E486C() { __dt__Q23std9exceptionFv(this, -1); }
};

#undef CONSTRUCT_PENDING_VOICE

} // extern "C"

namespace std {
struct length_error : ErrorBase_u802E486C {
    // Construct the copied message and install the length-error vtable.
    length_error(const char* msg) : ErrorBase_u802E486C(msg) { vtable = __vt__Q23std12length_error; }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}

extern "C" {

// Construct and throw a vector-length error through the runtime cleanup entry.
inline void throwLength(const char* msg) {
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(msg);
        exception_runtime::__throw(exception_runtime::voiceManagerStrings + 0x14, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Construct one pending record in uninitialized storage, deep-copying an owned
// filename. sourceEntry must be stable and dst may be null (checked by the caller).
// Statement macro deliberately retains the caller's exception/allocator-byte scope;
// inline extraction changes MWCC stack slots. This is the sole copy definition.
#define CONSTRUCT_PENDING_VOICE(destinationEntry, sourceEntry) \
    Str* destinationName = &(destinationEntry)->name; \
    Str* sourceName = &(sourceEntry)->name; \
    u32 flags = sourceName->flag; \
    if (!(flags >> 31)) { \
        u32 lengthWord = sourceName->size; \
        destinationName->flag = flags; \
        destinationName->size = lengthWord; \
        destinationName->ptr = sourceName->ptr; \
    } else { \
        destinationName->flag = 0; \
        destinationName->size = 0; \
        destinationName->ptr = 0; \
        String_ReserveCapacity(destinationName, sourceName->size); \
        Alloc allocator; \
        Alloc copy = allocator; \
        String_ReplaceRange(destinationName, 0, 0, sourceName->ptr, sourceName->ptr + sourceName->size, &copy); \
    } \
    (destinationEntry)->delaySeconds = (sourceEntry)->delaySeconds; \
    /* end pending-record construction */

#pragma opt_common_subs off

// Append the normal-power club category; powered modes add no candidate.
// Filename indices and candidate vector are borrowed; singleton calls are refreshed.
static inline void appendNormalClubCandidate(void* self, OutVec* candidateNames, VoiceFilenameIndexWord characterFilenameIndex, VoiceFilenameIndexWord caddieFilenameIndex) {
void* state = InstanceTable_Get();
        if (((VoiceClubSelection*)state)->powerMode != kNormalPowerMode) return;
        state = InstanceTable_Get();
        unsigned char x = ((VoiceClubSelection*)state)->selected->clubIndex;
        if (x > kLastLongClubVoiceIndex) CVoiceManager_AppendFormattedVoiceName(self, candidateNames, kVoiceShortClubOrOtherDoublePower, characterFilenameIndex, caddieFilenameIndex);
        else CVoiceManager_AppendFormattedVoiceName(self, candidateNames, kVoiceLongClub, characterFilenameIndex, caddieFilenameIndex);
        return;
}

// Append a powered-club category based on power mode and wood/iron group.
// Filename indices and candidate vector are borrowed; singleton calls are refreshed.
static inline void appendPoweredClubCandidate(void* self, OutVec* candidateNames, VoiceFilenameIndexWord characterFilenameIndex, VoiceFilenameIndexWord caddieFilenameIndex) {
if (((VoiceClubSelection*)InstanceTable_Get())->powerMode == kPowerShotMode) {
            CVoiceManager_AppendFormattedVoiceName(self, candidateNames, kVoicePowerShotOrDoublePowerIron, characterFilenameIndex, caddieFilenameIndex);
            return;
        }
        if (((VoiceClubSelection*)InstanceTable_Get())->powerMode != kDoublePowerShotMode) return;
        void* state = InstanceTable_Get();
        if (((VoiceClubSelection*)state)->selected->clubGroup == kClubGroupWood) {
            CVoiceManager_AppendFormattedVoiceName(self, candidateNames, kVoiceDoublePowerWood, characterFilenameIndex, caddieFilenameIndex);
            return;
        }
        state = InstanceTable_Get();
        if (((VoiceClubSelection*)state)->selected->clubGroup == kClubGroupIron) {
            CVoiceManager_AppendFormattedVoiceName(self, candidateNames, kVoicePowerShotOrDoublePowerIron, characterFilenameIndex, caddieFilenameIndex);
            return;
        }
        CVoiceManager_AppendFormattedVoiceName(self, candidateNames, kVoiceShortClubOrOtherDoublePower, characterFilenameIndex, caddieFilenameIndex);
        return;
}

// Append stroke-play hole-result categories for strokes minus par (integer strokes).
// One-stroke results are handled by the caller before this policy is reached.
static inline void appendStrokePlayScoreVoices(void* self, OutVec& candidateNames,
    VoiceFilenameIndexWord characterFilenameIndex, VoiceFilenameIndexWord caddieFilenameIndex, int strokesRelativeToPar) {
    switch (strokesRelativeToPar) {
    case kVoiceTwoUnderPar: // Eagle: two strokes under par.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceEagle, characterFilenameIndex, caddieFilenameIndex);
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceBirdieOrEagleAlternate, characterFilenameIndex, caddieFilenameIndex);
        break;
    case kVoiceOneUnderPar: // Birdie: one stroke under par.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceBirdie, characterFilenameIndex, caddieFilenameIndex);
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceBirdieOrEagleAlternate, characterFilenameIndex, caddieFilenameIndex);
        break;
    case kVoiceAtPar: // Par.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceParOrOtherHoleWin, characterFilenameIndex, caddieFilenameIndex);
        break;
    default:
        if (strokesRelativeToPar <= kVoiceThreeUnderPar) {
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceSingleStrokeOrAtLeastThreeUnderPar, characterFilenameIndex, caddieFilenameIndex);
        } else {
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceOverParOrHoleLoss, characterFilenameIndex, caddieFilenameIndex);
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceOverParAlternate, characterFilenameIndex, caddieFilenameIndex);
        }
        break;
    }
}

// Append a winning paired-opponent result; par or over-par wins share one category.
static inline void appendOpponentWinScoreVoice(void* self, OutVec& candidateNames,
    VoiceFilenameIndexWord characterFilenameIndex, VoiceFilenameIndexWord caddieFilenameIndex, int strokesRelativeToPar) {
    switch (strokesRelativeToPar) {
    case kVoiceTwoUnderPar: // Eagle: two strokes under par.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceEagle, characterFilenameIndex, caddieFilenameIndex);
        break;
    case kVoiceOneUnderPar: // Birdie: one stroke under par.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceBirdie, characterFilenameIndex, caddieFilenameIndex);
        break;
    default:
        if (strokesRelativeToPar <= kVoiceThreeUnderPar) CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceSingleStrokeOrAtLeastThreeUnderPar, characterFilenameIndex, caddieFilenameIndex);
        else CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceParOrOtherHoleWin, characterFilenameIndex, caddieFilenameIndex);
        break;
    }
}

// Resolve the round-result speaker and success flag for modes 0/6 or ranked players.
// Index outputs are filename selectors; caller-owned scan temporaries preserve registers.
static inline void resolveRoundVoiceSpeaker(VoiceFilenameIndexWord& characterFilenameIndex, VoiceFilenameIndexWord& caddieFilenameIndex,
    int& valid, u32 mode, int& playerIndex, int& equipmentOffset) {
    if (mode == kVoiceResultMode6) {
        characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->character);
        caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->caddie);
        if (AsCSharedDoc(g_sharedDoc)->partyGameMode != 0) {
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyResultTier) == 0) valid = 0;
        } else {
            if (VOICE_PLAYER_RESULT_NIBBLE(0) != 1u) valid = 0;
        }
    } else if (mode == kVoiceResultMode0) {
        characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->character);
        caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->caddie);
        switch (AsCSharedDoc(g_sharedDoc)->golfRuleType) {
        case kVoiceStrokeRule: // Rule zero accepts either of the first two result tiers.
            if (VOICE_PLAYER_RESULT_NIBBLE(0) > 2u) valid = 0;
            break;
        case kVoiceOpponentRule: // Opponent-rule success requires result tier one.
            if (VOICE_PLAYER_RESULT_NIBBLE(0) != 1u) valid = 0;
            break;
        default:
            break;
        }
    } else {
        char* players;
        for (playerIndex = 0; players = VOICE_GOLF_DOCUMENT_BYTES, playerIndex < ((CGolfDoc*)players)->playerCount; playerIndex++) {
            if (((*(unsigned char*)((char*)GolfPlayer_GetEntryByIndex(players, (unsigned char)playerIndex) + 0xc4) >> 4) & 0xf) == 1u) {
                equipmentOffset = playerIndex * kVoiceEquipmentStride;
                characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)(VOICE_DOCUMENT_BYTES + equipmentOffset))->character);
                caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)(VOICE_DOCUMENT_BYTES + equipmentOffset))->caddie);
                break;
            }
        }
    }
}

// Bit-preserving position copies use the original three-word aggregate transfer.
struct VoicePositionBits { u32 words[3]; };

// Collect first-stroke or horizontal-distance candidates; set seconds until playback.
// Candidate storage and pending entry are caller-owned; selectors are borrowed ABI words.
static inline void appendStrokeDistanceCandidates(void* self, OutVec& candidateNames, Entry& pendingVoice, VoiceFilenameIndexWord characterFilenameIndex, VoiceFilenameIndexWord caddieFilenameIndex) {
    // Reverse declaration order retains the original stack offsets after inlining.
    Vec3 holePosition;
    Vec3 horizontalHoleDelta;

    do {
        if (CURRENT_VOICE_HOLE_STROKES == 1) {
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceFirstStrokePrimary, characterFilenameIndex, caddieFilenameIndex);
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceFirstStrokeAlternate, characterFilenameIndex, caddieFilenameIndex);
        } else {
            char* currentPlayerSnapshotBytes;
            char* golfDocumentBytes;
            char* holeRecordBytes;
            currentPlayerSnapshotBytes = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer);
            golfDocumentBytes = VOICE_GOLF_DOCUMENT_BYTES;
            holeRecordBytes = ((char*)((CGolfDoc*)golfDocumentBytes)->holeRecords) + ((CGolfDoc*)golfDocumentBytes)->currentHoleIndex * kVoiceHoleRecordBytes;
            *(VoicePositionBits*)&holePosition = *(VoicePositionBits*)&(((VoiceHoleRecord*)holeRecordBytes)->position);
            __mi__FRC7WVectorRC7WVector(&horizontalHoleDelta, &holePosition, ((GolfDocSnapshot8013*)currentPlayerSnapshotBytes)->ballPosition);
            horizontalHoleDelta.y = 0.0f;
            float distanceYards = WVectorLen__FRC7WVector(&horizontalHoleDelta) / 3.2f;
            if (distanceYards > 200.0f) CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceDistanceOver200Yards, characterFilenameIndex, caddieFilenameIndex);
            else if (distanceYards > 50.0f) CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceDistanceOver50Yards, characterFilenameIndex, caddieFilenameIndex);
        }
        pendingVoice.delaySeconds = 1.0f;
        break;
    } while (0);
}

// Collect terrain/travel response names; all positions and thresholds are world units.
// Candidate storage and pending entry are caller-owned; selectors are borrowed ABI words.
static inline void appendLandingCandidates(void* self, OutVec& candidateNames, Entry& pendingVoice, VoiceFilenameIndexWord characterFilenameIndex, VoiceFilenameIndexWord caddieFilenameIndex) {
    // Reverse declaration order retains the original stack offsets after inlining.
    Vec3 currentHolePosition;
    Vec3 currentHoleDelta;
    Vec3 preShotHolePosition;
    Vec3 preShotHoleDelta;
    Vec3 landingHolePosition;
    Vec3 landingHoleDelta;
    Vec3 firstStrokeDelta;
    Vec3 preShotPosition;
    Vec3 landingPosition;

    do {
        *(VoicePositionBits*)&landingPosition = *(VoicePositionBits*)&(*(Vec3*)GolfBall__Fv());
        int currentGroundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &landingPosition);
        *(VoicePositionBits*)&preShotPosition = *(VoicePositionBits*)&(*(Vec3*)(char*)PreData__9CGolfBallFi(GolfBall__Fv(), 0));
        int previousGroundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &preShotPosition);

        pendingVoice.delaySeconds = 1.0f;
        if (((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->tutorialStage < 5) break;

        if (*(unsigned char*)((char*)GolfBall__Fv() + 0x58) != 0 ||
            (unsigned char)currentGroundType == 3u || (unsigned char)currentGroundType == 12u ||
            (unsigned char)currentGroundType == 10u) {
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceLandingSpecialCase25, characterFilenameIndex, caddieFilenameIndex);
            pendingVoice.delaySeconds = 1.3f;
            break;
        }
        if ((unsigned char)currentGroundType > 3) break;
        if ((unsigned char)previousGroundType == 2) break;

        signed char currentHoleStrokes;
        char* golfDocumentBytes;
        unsigned char currentHoleIndex;
        {
            char* playerSnapshotBytes = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer);
            golfDocumentBytes = VOICE_GOLF_DOCUMENT_BYTES;
            currentHoleIndex = ((CGolfDoc*)golfDocumentBytes)->currentHoleIndex;
            currentHoleStrokes = ((GolfDocSnapshot8013*)playerSnapshotBytes)->holeStrokeCounts[currentHoleIndex];
        }

        if (currentHoleStrokes == 1) {
            void* landingPositionPointer = GolfBall__Fv();
            char* preShotPositionBytes = (char*)PreData__9CGolfBallFi(GolfBall__Fv(), 0);
            __mi__FRC7WVectorRC7WVector(&firstStrokeDelta, preShotPositionBytes, landingPositionPointer);
            float len = WVectorLen__FRC7WVector(&firstStrokeDelta);
            if ((unsigned char)currentGroundType == 2 || len > 672.0f) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceStrongLandingResponse, characterFilenameIndex, caddieFilenameIndex);
                break;
            }
            if (len > 576.0f) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceModerateLandingResponse, characterFilenameIndex, caddieFilenameIndex);
                break;
            }
            break;
        }

        if ((unsigned char)currentGroundType == 2) {
            char* scorePlayer = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer);
            char* scorePlayers = VOICE_GOLF_DOCUMENT_BYTES;
            unsigned char scoreIndex = ((CGolfDoc*)scorePlayers)->currentHoleIndex;
            char* scoreRec = ((char*)((CGolfDoc*)scorePlayers)->holeRecords) + scoreIndex * kVoiceHoleRecordBytes;
            if (((GolfDocSnapshot8013*)scorePlayer)->holeStrokeCounts[scoreIndex] < ((VoiceHoleRecord*)scoreRec)->par - 2) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceStrongLandingResponse, characterFilenameIndex, caddieFilenameIndex);
                break;
            }
            *(VoicePositionBits*)&landingHolePosition = *(VoicePositionBits*)&(((VoiceHoleRecord*)scoreRec)->position);
            __mi__FRC7WVectorRC7WVector(&landingHoleDelta, &landingHolePosition, GolfBall__Fv());
            float len = WVectorLen__FRC7WVector(&landingHoleDelta);
            char* playerSnapshotBytes = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer);
            golfDocumentBytes = VOICE_GOLF_DOCUMENT_BYTES;
            currentHoleIndex = ((CGolfDoc*)golfDocumentBytes)->currentHoleIndex;
            char* rec = ((char*)((CGolfDoc*)golfDocumentBytes)->holeRecords) + currentHoleIndex * kVoiceHoleRecordBytes;
            if (((GolfDocSnapshot8013*)playerSnapshotBytes)->holeStrokeCounts[currentHoleIndex] < ((VoiceHoleRecord*)rec)->par && len < 19.2f) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceStrongLandingResponse, characterFilenameIndex, caddieFilenameIndex);
                break;
            }
            playerSnapshotBytes = (char*)GolfPlayer_GetEntryByIndex(golfDocumentBytes, ((CGolfDoc*)golfDocumentBytes)->currentPlayer);
            golfDocumentBytes = VOICE_GOLF_DOCUMENT_BYTES;
            currentHoleIndex = ((CGolfDoc*)golfDocumentBytes)->currentHoleIndex;
            rec = ((char*)((CGolfDoc*)golfDocumentBytes)->holeRecords) + currentHoleIndex * kVoiceHoleRecordBytes;
            if (((GolfDocSnapshot8013*)playerSnapshotBytes)->holeStrokeCounts[currentHoleIndex] < ((VoiceHoleRecord*)rec)->par && len < 38.4f) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceModerateLandingResponse, characterFilenameIndex, caddieFilenameIndex);
                break;
            }
        }

        {
            *(VoicePositionBits*)&preShotHolePosition = *(VoicePositionBits*)&(*(Vec3*)(((char*)((CGolfDoc*)golfDocumentBytes)->holeRecords) + currentHoleIndex * kVoiceHoleRecordBytes + 0xc));
            __mi__FRC7WVectorRC7WVector(&preShotHoleDelta, &preShotHolePosition, (char*)PreData__9CGolfBallFi(GolfBall__Fv(), 0));
            float preShotDistanceToHole = WVectorLen__FRC7WVector(&preShotHoleDelta);

            *(VoicePositionBits*)&currentHolePosition = *(VoicePositionBits*)&(*(Vec3*)((*(char**)&((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->holeRecords) + ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex * kVoiceHoleRecordBytes + 0xc));
            __mi__FRC7WVectorRC7WVector(&currentHoleDelta, &currentHolePosition, GolfBall__Fv());
            float landingDistanceToHole = WVectorLen__FRC7WVector(&currentHoleDelta);
            if (preShotDistanceToHole - landingDistanceToHole > 384.0f) CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceModerateLandingResponse, characterFilenameIndex, caddieFilenameIndex);
        }
        break;
    } while (0);
}

// Release all candidate-owned filename buffers and then the candidate allocation.
// Scan temporaries belong to the caller to retain MWCC register assignment.
static inline void releaseVoiceCandidates(OutVec& candidateNames, Str*& candidateToDestroy, int& candidatesToDestroy, volatile int& eraseAlloc) {
    if (candidateNames.begin != 0) {
        candidatesToDestroy = candidateNames.size;
        *(volatile char*)&eraseAlloc = 0;
        candidateToDestroy = candidateNames.begin + candidatesToDestroy;
        candidateNames.size -= candidatesToDestroy;
        while (candidatesToDestroy != 0) {
            candidateToDestroy--;
            if (candidateToDestroy != 0 && (candidateToDestroy->flag >> 31)) appFreeUni__FPv(candidateToDestroy->ptr);
            candidatesToDestroy--;
        }
        appFreeUni__FPv(candidateNames.begin);
    }
}

// arg is a borrowed gameplay message whose event code is the word at +4.
// The document, golf state and inventory must be initialized. Character/caddie
// indices select filename variants; they can be replaced by the winning player
// in result events. Each candidate is equally likely after collection. Empty
// collections do nothing; nonempty ones deep-copy one name into the pending queue.
// Build voice candidates for the gameplay message, choose a sample and queue its playback.
void CVoiceManager_HandleMsg(void* self, void* arg) {
    char* selectedFilenameChars;
    u32 pendingNameLengthBeforeCopy;
    char* sharedDocumentAlias;
    VoiceFilenameIndexWord characterFilenameIndex;
    VoiceFilenameIndexWord caddieFilenameIndex;
    int unusedLegacyIndex;
    Entry* entryDestination;
    u32 grownEntryCapacity;
    Entry* oldEntriesBegin;
    Entry* oldEntriesEnd;
    u32 entriesToConstruct;
    Str* candidateToDestroy;
    int candidatesToDestroy;

    OutVec candidateNames;
    Entry pendingVoice;

    candidateNames.begin = 0;
    candidateNames.size = 0;
    candidateNames.cap = 0;
    pendingVoice.name.flag = 0;
    pendingVoice.name.size = 0;
    pendingVoice.name.ptr = 0;
    pendingVoice.delaySeconds = 0.0f;

    // Phase 1: resolve the current player's character/caddie filename indices.
    sharedDocumentAlias = (char*)g_sharedDoc;
    characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory,
        ((VoiceEquipmentSlot*)(VOICE_DOCUMENT_BYTES + ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer * kVoiceEquipmentStride))->character);
    caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory,
        ((VoiceEquipmentSlot*)(VOICE_DOCUMENT_BYTES + ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer * kVoiceEquipmentStride))->caddie);

    // Phase 2: event-specific candidates and delay. Codes are the message ABI:
    // 0 opening (1.5s); 1 normal club (0s); 2 powered club (0s); 3 code-22 (1s);
    // 4 stroke start/distance (1s); 5 landing/ground response (1s, or 1.3s);
    // 6 hole result (1.5s); 7 overall result (5s); 8 code-26 (1.5s);
    // 9 result qualifier (1s); 10 code-7 (0s). Exact meanings of 3/8/10 unresolved.
    switch (*(unsigned int*)((char*)arg + 4)) {
    case kVoiceOpeningEvent: // Queue both opening candidates with an opening delay.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceOpeningPrimary, characterFilenameIndex, caddieFilenameIndex);
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceOpeningAlternate, characterFilenameIndex, caddieFilenameIndex);
        pendingVoice.delaySeconds = 1.5f;
        break;

    case kVoiceNormalClubEvent: // Collect the normal-power club category.
        appendNormalClubCandidate(self, &candidateNames, characterFilenameIndex, caddieFilenameIndex);
        break;

    case kVoicePoweredClubEvent: // Collect the power-shot club category.
        appendPoweredClubCandidate(self, &candidateNames, characterFilenameIndex, caddieFilenameIndex);
        break;

    case kVoiceUnresolvedEvent3: // Queue category 22; spoken/event meaning is unresolved.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceUnresolvedEvent3Category, characterFilenameIndex, caddieFilenameIndex);
        pendingVoice.delaySeconds = 1.0f;
        break;

    // Distances divide world XZ units by 3.2 to obtain yards for 50/200-yard tests.
    case kVoiceStrokeDistanceEvent: { // Collect first-stroke or horizontal-distance candidates.
        appendStrokeDistanceCandidates(self, candidateNames, pendingVoice, characterFilenameIndex, caddieFilenameIndex);
        break;
    }

    // Compare current and pre-shot terrain/positions; thresholds below are world units.
    case kVoiceLandingEvent: { // Choose a response from terrain and shot travel.
        appendLandingCandidates(self, candidateNames, pendingVoice, characterFilenameIndex, caddieFilenameIndex);
        break;
    }

    // Hole result: rule 0 compares strokes with par; rule 1 compares opposing slots.
    case kVoiceHoleResultEvent: { // Choose stroke/par or paired-opponent hole-result policy.
        switch (AsCSharedDoc(g_sharedDoc)->golfRuleType) {
        case kVoiceStrokeRule: { // Compare strokes against par.
            unsigned char currentHoleStrokes = CURRENT_VOICE_HOLE_STROKES;
            if (currentHoleStrokes == 1) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceSingleStrokeOrAtLeastThreeUnderPar, characterFilenameIndex, caddieFilenameIndex);
            } else {
                char* golfDocumentBytes;
                unsigned char holeIndex;
                char* currentPlayerRecord = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer);
                golfDocumentBytes = VOICE_GOLF_DOCUMENT_BYTES;
                holeIndex = ((CGolfDoc*)golfDocumentBytes)->currentHoleIndex;
                unsigned char holeStrokes = ((GolfDocSnapshot8013*)currentPlayerRecord)->holeStrokeCounts[holeIndex];
                char* holeRecord = ((char*)((CGolfDoc*)golfDocumentBytes)->holeRecords) + holeIndex * kVoiceHoleRecordBytes;
                int strokesRelativeToPar = (int)holeStrokes - (int)((VoiceHoleRecord*)holeRecord)->par;
                appendStrokePlayScoreVoices(self, candidateNames, characterFilenameIndex, caddieFilenameIndex, strokesRelativeToPar);
            }
            break;
        }
        case kVoiceOpponentRule: { // Compare paired slots; modes 0/6 voice player zero.
            unsigned char currentSlot = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentPlayer;
            unsigned int resultGameMode = AsCSharedDoc(g_sharedDoc)->gameMode;
            unsigned char opponentSlot = currentSlot ^ kPairedOpponentSlotBit;
            if (resultGameMode == kVoiceResultMode0 || resultGameMode == kVoiceResultMode6) {
                characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->character);
                caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->caddie);
                currentSlot = 0;
                opponentSlot = 1;
            }
            unsigned char opponentHoleIndex = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex;
            char* opponentRecord = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, opponentSlot);
            unsigned char currentHoleIndex = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex;
            char* currentRecord = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, currentSlot);
            unsigned char opponentStrokes = ((GolfDocSnapshot8013*)opponentRecord)->holeStrokeCounts[opponentHoleIndex];
            unsigned char currentStrokes = ((GolfDocSnapshot8013*)currentRecord)->holeStrokeCounts[currentHoleIndex];
            if (currentStrokes < opponentStrokes) {
                unsigned char holeIndex = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex;
                char* player = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, currentSlot);
                unsigned char currentHoleStrokes = ((GolfDocSnapshot8013*)player)->holeStrokeCounts[holeIndex];
                if (currentHoleStrokes == 1) {
                    CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceSingleStrokeOrAtLeastThreeUnderPar, characterFilenameIndex, caddieFilenameIndex);
                } else {
                    unsigned char scoreIndex = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex;
                    char* scorePlayer = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, currentSlot);
                    unsigned char holeStrokes = ((GolfDocSnapshot8013*)scorePlayer)->holeStrokeCounts[scoreIndex];
                    char* holeRecord = *(char**)&((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->holeRecords + ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex * kVoiceHoleRecordBytes;
                    int strokesRelativeToPar = (int)holeStrokes - (int)((VoiceHoleRecord*)holeRecord)->par;
                    appendOpponentWinScoreVoice(self, candidateNames, characterFilenameIndex, caddieFilenameIndex, strokesRelativeToPar);
                }
            } else {
                unsigned char opponentHoleIndex = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex;
                char* opponentRecord = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, opponentSlot);
                unsigned char currentHoleIndex = ((CGolfDoc*)VOICE_GOLF_DOCUMENT_BYTES)->currentHoleIndex;
                char* currentRecord = (char*)GolfPlayer_GetEntryByIndex(VOICE_GOLF_DOCUMENT_BYTES, currentSlot);
                unsigned char opponentStrokes = ((GolfDocSnapshot8013*)opponentRecord)->holeStrokeCounts[opponentHoleIndex];
                unsigned char currentStrokes = ((GolfDocSnapshot8013*)currentRecord)->holeStrokeCounts[currentHoleIndex];
                if (currentStrokes > opponentStrokes) CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceOverParOrHoleLoss, characterFilenameIndex, caddieFilenameIndex);
            }
            break;
        }
        default:
            break;
        }
        pendingVoice.delaySeconds = 1.5f;
        break;
    }

    case kVoiceRoundResultEvent: { // Resolve the speaker, then voice overall round success or failure.
        int speakerResultQualified = 1;
        u32 resultGameMode = AsCSharedDoc(g_sharedDoc)->gameMode;
        int playerIndex, equipmentOffset;
        resolveRoundVoiceSpeaker(characterFilenameIndex, caddieFilenameIndex, speakerResultQualified, resultGameMode, playerIndex, equipmentOffset);

        if (speakerResultQualified) {
            u32 resultGameMode;
            char* sharedDocumentBytes;
            sharedDocumentBytes = VOICE_DOCUMENT_BYTES;
            resultGameMode = AsCSharedDoc(sharedDocumentBytes)->gameMode;
            if ((resultGameMode == kVoiceResultMode6 && AsCSharedDoc(sharedDocumentBytes)->partyGameMode != 0) ||
                (resultGameMode == kVoiceResultMode0 && AsCSharedDoc(sharedDocumentBytes)->golfRuleType == 0 &&
                ((*(unsigned char*)((char*)GolfPlayer_GetEntryByIndex(((char*)AsCSharedDoc(sharedDocumentBytes)->golfDoc), 0) + 0xc4) >> 4) & 0xf) == 2u)) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceQualifiedRoundSuccess, characterFilenameIndex, caddieFilenameIndex);
            }
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceRoundSuccessAlternate, characterFilenameIndex, caddieFilenameIndex);
        } else {
            u32 resultGameMode;
            char* sharedDocumentBytes;
            sharedDocumentBytes = VOICE_DOCUMENT_BYTES;
            resultGameMode = AsCSharedDoc(sharedDocumentBytes)->gameMode;
            if ((resultGameMode == kVoiceResultMode6 && AsCSharedDoc(sharedDocumentBytes)->partyGameMode != 0) ||
                (resultGameMode == kVoiceResultMode0 && AsCSharedDoc(sharedDocumentBytes)->golfRuleType == 0 &&
                ((*(unsigned char*)((char*)GolfPlayer_GetEntryByIndex(((char*)AsCSharedDoc(sharedDocumentBytes)->golfDoc), 0) + 0xc4) >> 4) & 0xf) == 2u)) {
                CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceQualifiedRoundFailure, characterFilenameIndex, caddieFilenameIndex);
            }
            CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceRoundFailureAlternate, characterFilenameIndex, caddieFilenameIndex);
        }
        pendingVoice.delaySeconds = 5.0f;
        break;
    }

    case kVoiceUnresolvedEvent8: // Queue category 26; spoken/event meaning is unresolved.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceUnresolvedEvent8Category, characterFilenameIndex, caddieFilenameIndex);
        pendingVoice.delaySeconds = 1.5f;
        break;

    case kVoiceResultQualifierEvent: { // Add a qualified result unless the round-result branch covers it.
        u32 initialMode;
        char* sharedDocumentBytes;
        sharedDocumentBytes = VOICE_DOCUMENT_BYTES;
        initialMode = AsCSharedDoc(sharedDocumentBytes)->gameMode;
        if ((initialMode == kVoiceResultMode6 && AsCSharedDoc(sharedDocumentBytes)->partyGameMode != 0) ||
            (initialMode == kVoiceResultMode0 && AsCSharedDoc(sharedDocumentBytes)->golfRuleType == 0 &&
            ((*(unsigned char*)((char*)GolfPlayer_GetEntryByIndex(((char*)AsCSharedDoc(sharedDocumentBytes)->golfDoc), 0) + 0xc4) >> 4) & 0xf) == 2u)) break;
        int speakerResultQualified = 1;
        u32 resultGameMode = AsCSharedDoc(g_sharedDoc)->gameMode;
        if (resultGameMode == kVoiceResultMode6) {
            characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->character);
            caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->caddie);
            if (VOICE_PLAYER_RESULT_NIBBLE(0) != 1u) speakerResultQualified = 0;
        } else if (resultGameMode == kVoiceResultMode0) {
            characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->character);
            caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)VOICE_DOCUMENT_BYTES)->caddie);
            switch (AsCSharedDoc(g_sharedDoc)->golfRuleType) {
            case kVoiceStrokeRule: // Accept either of the first two result tiers.
                if (VOICE_PLAYER_RESULT_NIBBLE(0) > 2u) speakerResultQualified = 0;
                break;
            case kVoiceOpponentRule: // Require the first result tier.
                if (VOICE_PLAYER_RESULT_NIBBLE(0) != 1u) speakerResultQualified = 0;
                break;
            default:
                break;
            }
        } else {
            char* golfDocumentBytes;
            for (int resultSpeakerSlot = 0; golfDocumentBytes = VOICE_GOLF_DOCUMENT_BYTES, resultSpeakerSlot < ((CGolfDoc*)golfDocumentBytes)->playerCount; resultSpeakerSlot++) {
                if (((*(unsigned char*)((char*)GolfPlayer_GetEntryByIndex(golfDocumentBytes, (unsigned char)resultSpeakerSlot) + 0xc4) >> 4) & 0xf) == 1u) {
                    int speakerEquipmentOffset = resultSpeakerSlot * kVoiceEquipmentStride;
                    characterFilenameIndex = ItemInventory_FindCharacterIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)(VOICE_DOCUMENT_BYTES + speakerEquipmentOffset))->character);
                    caddieFilenameIndex = ItemInventory_FindCaddieIndex(&AsCSharedDoc(g_sharedDoc)->itemInventory, ((VoiceEquipmentSlot*)(VOICE_DOCUMENT_BYTES + speakerEquipmentOffset))->caddie);
                    break;
                }
            }
        }
        if (speakerResultQualified) CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceQualifiedRoundSuccess, characterFilenameIndex, caddieFilenameIndex);
        else CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceQualifiedRoundFailure, characterFilenameIndex, caddieFilenameIndex);
        pendingVoice.delaySeconds = 1.0f;
        break;
    }

    case kVoiceUnresolvedEvent10: // Queue category 7 without a delay.
        CVoiceManager_AppendFormattedVoiceName(self, &candidateNames, kVoiceOverParOrHoleLoss, characterFilenameIndex, caddieFilenameIndex);
        break;

    default:
        break;
    }

    // Phase 3: choose uniformly from candidate names; no candidates means no queue entry.
    u32 candidateCount = candidateNames.size;
    if (candidateCount == 0) {
        if (pendingVoice.name.flag >> 31) appFreeUni__FPv(pendingVoice.name.ptr);
        volatile int eraseAlloc;
        releaseVoiceCandidates(candidateNames, candidateToDestroy, candidatesToDestroy, eraseAlloc);
        return;
    }

    {
        u32 selectedCandidateIndex = (u32)rand() % candidateCount;
        Str* selectedCandidate = candidateNames.begin + selectedCandidateIndex;
        selectedFilenameChars = !candidateNames.begin[selectedCandidateIndex].heapbit ? (char*)selectedCandidate + 1 : selectedCandidate->ptr;

        pendingNameLengthBeforeCopy = !pendingVoice.name.heapbit ? pendingVoice.name.smallsize : pendingVoice.name.size;
        Alloc filenameAllocator;
        Alloc filenameCopyAllocator = filenameAllocator;
        String_ReplaceRange(&pendingVoice.name, 0, pendingNameLengthBeforeCopy, selectedFilenameChars, selectedFilenameChars + strlen(selectedFilenameChars), &filenameCopyAllocator);
    }

    // Phase 4: deep-copy the pending entry in place, or grow/copy/exchange buffers.
    // PendingVoiceVector addresses 16-byte Entry records (name + delaySeconds);
    // OutVec candidateNames addresses 12-byte Str elements. All
    // pointer math below is therefore in Entry units (grownEntryCapacity * 16 bytes). The same name-copy block
    // (inline string: copy 3 words; heap string: reserve and copy characters, then copy delaySeconds)
    // appears three times: once for the new entry when capacity allows, once for the new entry in a
    // freshly grown buffer, and once per old entry when moving the old prefix (copied backward).
    // Growth: capacity += max(3/5 or 1/2 of (capacity + 1), needed extra), saturating at 0x0FFFFFFF.
    {
        PendingVoiceVector* pendingQueue = (PendingVoiceVector*)(((char*)&AsCVoiceManager(self)->pendingVoices));
        volatile int pendingAppendAllocatorTag;
        *(volatile char*)&pendingAppendAllocatorTag = 0;
        // Copy site A: existing allocation; the new entry is uninitialized storage.
        if (pendingQueue->size < pendingQueue->cap) {
            entryDestination = (Entry*)pendingQueue->begin + pendingQueue->size;
            do {
                if (entryDestination != 0) {
                    try {
                        CONSTRUCT_PENDING_VOICE(entryDestination, &pendingVoice);
                    } catch (...) { __throw(0, 0, 0); }
                }
            } while (0);
            pendingQueue->size = pendingQueue->size + 1;
        } else {
            volatile int insertScopeAllocatorTag;
            *(volatile char*)&insertScopeAllocatorTag = 0;
            volatile int vectorInsertAllocatorTag;
            *(volatile char*)&vectorInsertAllocatorTag = 0;
            if (kMaxPendingVoiceEntries - (u32)pendingQueue->cap < 1) throwLength(voiceManagerStrings);
            u32 extra = (u32)(pendingQueue->size + 1) - (u32)pendingQueue->cap;
            GrowEntry guard((unsigned int*)&pendingQueue->cap);
            grownEntryCapacity = (u32)pendingQueue->cap;
            if (extra > kMaxPendingVoiceEntries - grownEntryCapacity) throwLength(voiceManagerStrings);
            if (grownEntryCapacity < kPendingVoiceSmallGrowthLimit) grownEntryCapacity += growMax((grownEntryCapacity + 1) * 3 / 5, extra);
            else if (grownEntryCapacity < kPendingVoiceHalfGrowthLimit) grownEntryCapacity += growMax((grownEntryCapacity + 1) / 2, extra);
            else grownEntryCapacity = kMaxPendingVoiceEntries;
            volatile int growthScopeAllocatorTag;
            *(volatile char*)&growthScopeAllocatorTag = 0;
            if (grownEntryCapacity > kMaxPendingVoiceEntries) throwLength(voiceManagerStrings);
            Entry* newEntryStorage = (Entry*)appMallocDummy__FUl(grownEntryCapacity * sizeof(Entry));
            u32 oldEntryCount = (u32)pendingQueue->size;
            guard.data = newEntryStorage;
            guard.capacity = grownEntryCapacity;
            entryDestination = newEntryStorage + oldEntryCount + guard.size;
            guard.position = oldEntryCount;
            volatile int newEntryCopyAllocatorTag;
            *(volatile char*)&newEntryCopyAllocatorTag = 0;
            // Copy site B: construct the new last entry before touching the old prefix.
            entriesToConstruct = 1;
            while (entriesToConstruct != 0) {
                do {
                    if (entryDestination != 0) {
                        try {
                            CONSTRUCT_PENDING_VOICE(entryDestination, &pendingVoice);
                        } catch (...) { __throw(0, 0, 0); }
                    }
                } while (0);
                --entriesToConstruct;
                ++entryDestination;
                ++guard.size;
            }
            // Copy site C: copy old entries backward. Each successful copy expands
            // guard ownership; an exception destroys only the constructed suffix.
            oldEntriesBegin = (Entry*)pendingQueue->begin;
            oldEntriesEnd = oldEntriesBegin + pendingQueue->size;
            entryDestination = guard.data + guard.position;
            volatile int prefixCopyAllocatorTag;
            *(volatile char*)&prefixCopyAllocatorTag = 0;
            while (oldEntriesEnd > oldEntriesBegin) {
                --entryDestination;
                --oldEntriesEnd;
                do {
                    if (entryDestination != 0) {
                        try {
                            CONSTRUCT_PENDING_VOICE(entryDestination, oldEntriesEnd);
                        } catch (...) { __throw(0, 0, 0); }
                    }
                } while (0);
                --guard.position;
                ++guard.size;
            }
            // Publish the new vector. The guard takes the old strings and allocation.
            Entry* displacedEntries = (Entry*)pendingQueue->begin;
            Entry* replacementEntries = guard.data;
            u32 publishedEntryCount = guard.size;
            u32 displacedEntryCount = pendingQueue->size;
            u32 displacedCapacity = (u32)pendingQueue->cap;
            guard.data = displacedEntries;
            pendingQueue->cap = (int)guard.capacity;
            guard.capacity = displacedCapacity;
            pendingQueue->begin = replacementEntries;
            pendingQueue->size = publishedEntryCount;
            guard.size = displacedEntryCount;
        }
    }

    // Phase 5: discard local copies; the queued name owns independent string storage.
    if (pendingVoice.name.flag >> 31) appFreeUni__FPv(pendingVoice.name.ptr);
    volatile int eraseAlloc;
    releaseVoiceCandidates(candidateNames, candidateToDestroy, candidatesToDestroy, eraseAlloc);
    (void)unusedLegacyIndex;
}

#pragma opt_common_subs reset

#undef CONSTRUCT_PENDING_VOICE

} // extern "C"

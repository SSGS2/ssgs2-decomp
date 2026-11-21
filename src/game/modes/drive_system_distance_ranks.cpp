#include <decomp/static_init.h>
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include <decomp/cw_bitfields.h>
// Translation unit 0x802E43A8-0x802E486C (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern char s_driveSystemTypeInfo;
extern float kFloat_0_NumericMenuInput;
extern float kFloat_6_2831855;
extern float s_driveSystemFullTurnAngle;
extern void* __nw__FUl(unsigned int);
extern void __ct__18NativeBase80201D18Fv(void*);
extern char __vt__13CVoiceManager[];
extern void* g_sharedDoc;
struct Player802E {
    char pad0[0xC];
    // Orders equal-distance ranks; a player-identity role is not established here.
    unsigned char rankTieBreakOrder;
    char padD[0xC4 - 0xD];
    CW_BITS2(
        unsigned char orderedRank : 4, // High nibble: 1 + all opponents ahead, including tie-break order.
        unsigned char tiedRank : 4) // Low nibble: equal distances retain the same rank.
    char padC5[0x178 - 0xC5];
    float distance;
    char pad17C[0x1C4 - 0x17C];
    float previousDistance;
};
extern Player802E* GolfPlayer_GetEntryByIndex(void*, unsigned int);
// Return the runtime type descriptor used for drive-contest dispatch.
void* CDriveSystem_GetRTTI() {
    return &s_driveSystemTypeInfo;
}

// Initialize the drive system angle constant during static startup.
void DriveSystem_InitFullTurnAngle() {
    s_driveSystemFullTurnAngle = kFloat_6_2831855;
}

struct Obj802E {
    unsigned int vtable;
    char nativeBaseState[0x14];
    void* voiceList;
    unsigned int voiceCount;
    unsigned int voiceCapacity;
    // Initialize the native base and an empty owned voice-list allocation.
    Obj802E() {
        __ct__18NativeBase80201D18Fv(this);
        vtable = (unsigned int)__vt__13CVoiceManager;
        voiceList = 0;
        voiceCount = 0;
        voiceCapacity = 0;
    }
};

// Allocate the voice manager and initialize its empty voice-list storage.
Obj802E* CVoiceManager_Create() {
    Obj802E* p = new Obj802E;
    return p;
}

// Borrow the active golf document; no reference is acquired.
static inline void* DriveGolfDoc() { return (void*)((CSharedDoc*)g_sharedDoc)->golfDoc; }

enum DriveRankValue { kFirstPlace = 1, kSecondPlace = 2, kThirdPlace = 3, kIneligibleDistanceRank = -1 };
// partyResultTier is a result category, not itself a rank: 1 -> second, 2/3 -> first.
// The distinction between categories 2 and 3 is unresolved in this unit.
enum DriveSinglePlayerTier { kTierAwardsSecond = 1, kTierAwardsFirstA = 2, kTierAwardsFirstB = 3 };

// Snapshot distances and publish tied/ordered ranks for the drive contest.
// Requires 1..4 players (fixed scratch arrays). Distances use existing course units.
// A sole eligible leader is promoted first; remaining comparisons include negative
// distances too. Only orderedRank breaks equal distances using the legacy id byte.
void CDriveSystem_UpdateDistanceRanks() {
    int count = ((CGolfDoc*)DriveGolfDoc())->playerCount;
    int i;
    int j;
    int k;

    // Phase 1: snapshot every distance before changing any rank nibble.
    for (i = 0; i < count; i++) {
        void* a = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
        void* b = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
        float distance = ((Player802E*)a)->distance;
        ((Player802E*)b)->previousDistance = distance;
    }

    // Phase 2a: single-player awards come from the party-result category.
    if (count == 1) {
        int resultTier = ((CSharedDoc*)g_sharedDoc)->partyResultTier;
        void* obj0 = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
        ((Player802E*)obj0)->orderedRank = kThirdPlace;

        switch (resultTier) {
        case kTierAwardsFirstA:
        case kTierAwardsFirstB: {
            void* o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
            ((Player802E*)o)->orderedRank = kFirstPlace;
            break;
        }
        case kTierAwardsSecond: {
            void* o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
            ((Player802E*)o)->orderedRank = kSecondPlace;
            break;
        }
        }

        void* src = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
        void* dst = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
        ((Player802E*)dst)->tiedRank = ((Player802E*)src)->orderedRank;
    } else {
        float minimumEligibleDistance = kFloat_0_NumericMenuInput;
        int soleLeader[4] = {0, 0, 0, 0};
        int distanceRank[4];

        // Phase 2b: -1 excludes negative distances; otherwise count strictly
        // farther eligible opponents. Zero therefore marks every eligible leader.
        int unranked = kIneligibleDistanceRank;
        for (j = 0; j < count; j++) {
            distanceRank[j] = unranked;
            void* obj = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
            float playerDistance = ((Player802E*)obj)->distance;
            if (!(playerDistance < minimumEligibleDistance)) {
                distanceRank[j] = distanceRank[j] + 1;
                for (k = 0; k < count; k++) {
                    void* objj = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)k);
                    float distance = ((Player802E*)objj)->distance;
                    if (j == k) continue;
                    if (distance < minimumEligibleDistance) continue;
                    if (!(playerDistance < distance)) continue;
                    distanceRank[j]++;
                }
            }
        }

        // Phase 3: starting at -1 makes zero mean EXACTLY ONE leader.
        // Tied leaders receive no sole-leader preference; no eligible leader gives -1.
        int eligibleLeaderCountMinusOne = -1;
        for (k = 0; k < count; k++) {
            if (distanceRank[k] == 0) eligibleLeaderCountMinusOne++;
        }

        for (k = 0; k < count; k++) {
            if (distanceRank[k] == 0 && eligibleLeaderCountMinusOne == 0) {
                soleLeader[k]++;
            }
        }

        // Phase 4: both ranks start at one. Each preferred/farther opponent
        // increments both; a smaller equal-distance tie-break key increments only
        // orderedRank. These are competition ranks (ties can leave rank gaps).
        int firstRank = kFirstPlace;
        for (i = 0; i < count; i++) {
            void* obji = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
            ((Player802E*)obji)->orderedRank = kFirstPlace;

            void* obji2 = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
            ((Player802E*)obji2)->tiedRank = kFirstPlace;

            for (j = 0; j < ((CGolfDoc*)DriveGolfDoc())->playerCount; j++) {
                if (j != i) {
                    if (soleLeader[i] < soleLeader[j]) {
                        void* o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                        ((Player802E*)o)->orderedRank++;

                        o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                        ((Player802E*)o)->tiedRank++;
                    } else if (soleLeader[i] == soleLeader[j]) {
                        void* objj = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                        void* obji3 = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                        if (((Player802E*)obji3)->distance < ((Player802E*)objj)->distance) {
                            void* o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                            ((Player802E*)o)->orderedRank++;

                            o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                            ((Player802E*)o)->tiedRank++;
                        } else {
                            objj = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                            obji3 = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                            if (((Player802E*)obji3)->distance == ((Player802E*)objj)->distance) {
                                void* oj = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                                void* oi = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                                unsigned char otherTieBreakOrder = ((Player802E*)oj)->rankTieBreakOrder;
                                unsigned char tieBreakOrder = ((Player802E*)oi)->rankTieBreakOrder;
                                if (tieBreakOrder > otherTieBreakOrder) {
                                    void* o = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                                    ((Player802E*)o)->orderedRank++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462574", _ctors_u802E43A8_0, DriveSystem_InitFullTurnAngle);
#pragma pop

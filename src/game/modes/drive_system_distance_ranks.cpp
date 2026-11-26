// Drive contest support: the drive system's RTTI and full-turn angle, and the
// distance ranking that publishes each player's tied and ordered place. The
// voice manager constructor also lives here.
#include <decomp/static_init.h>
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include <decomp/cw_bitfields.h>
#include "classes/CDriveSystem.h"

extern "C" {

extern char s_driveSystemTypeInfo;
extern float kFloat_0_NumericMenuInput;
extern float kFloat_6_2831855;
extern float s_driveSystemFullTurnAngle;
extern void* __nw__FUl(unsigned int);
extern void __ct__18NativeBase80201D18Fv(void*);
extern char __vt__13CVoiceManager[];
extern void* g_sharedDoc;
// Fields of a golf player's record that this file reads. They belong in the
// class header that owns it.
struct Player802E {
    char pad0[0xC];
    // Order used to break equal distances.
    unsigned char rankTieBreakOrder;
    char padD[0xC4 - 0xD];
    CW_BITS2(unsigned char orderedRank : 4, // High nibble: 1 + all opponents ahead, including tie-break order.
             unsigned char tiedRank : 4)    // Low nibble: equal distances retain the same rank.
    char padC5[0x178 - 0xC5];
    float distance;
    char pad17C[0x1C4 - 0x17C];
    float previousDistance;
};
extern Player802E* GolfPlayer_GetEntryByIndex(void*, unsigned int);
// Return the runtime type descriptor used for drive-contest dispatch.
void* CDriveSystem::GetRTTI() {
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
static inline void* DriveGolfDoc() {
    return (void*)((CSharedDoc*)g_sharedDoc)->golfDoc;
}

enum DriveRankValue {
    kFirstPlace = 1,
    kSecondPlace = 2,
    kThirdPlace = 3,
    kIneligibleDistanceRank = -1
};
// partyResultTier is a result category, not a rank: 1 gives second place,
// 2 and 3 give first place.
enum DriveSinglePlayerTier {
    kTierAwardsSecond = 1,
    kTierAwardsFirstA = 2,
    kTierAwardsFirstB = 3
};

} // extern "C"

// Snapshot distances and publish tied/ordered ranks for the drive contest.
// Requires 1..4 players (fixed scratch arrays). A sole eligible leader is
// promoted first; remaining comparisons include negative distances too. Only
// orderedRank breaks equal distances, by the lower tie-break byte.
void CDriveSystem::UpdateDistanceRanks() {
    int count = ((CGolfDoc*)DriveGolfDoc())->playerCount;
    int i;
    int j;
    int k;

    // Snapshot every distance before any rank nibble changes.
    for (i = 0; i < count; i++) {
        Player802E* sourceEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
        Player802E* destinationEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
        float distance = sourceEntry->distance;
        destinationEntry->previousDistance = distance;
    }

    // A single player is placed by the party result category; every other
    // category takes third place.
    if (count == 1) {
        int resultTier = ((CSharedDoc*)g_sharedDoc)->partyResultTier;
        Player802E* playerEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
        playerEntry->orderedRank = kThirdPlace;

        switch (resultTier) {
        case kTierAwardsFirstA:
        case kTierAwardsFirstB: {
            Player802E* entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
            entry->orderedRank = kFirstPlace;
            break;
        }
        case kTierAwardsSecond: {
            Player802E* entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
            entry->orderedRank = kSecondPlace;
            break;
        }
        }

        Player802E* rankedSource = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
        Player802E* rankedDestination = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), 0);
        rankedDestination->tiedRank = rankedSource->orderedRank;
    } else {
        float minimumEligibleDistance = kFloat_0_NumericMenuInput;
        int soleLeader[4] = {0, 0, 0, 0};
        int distanceRank[4];

        // Start at -1 so a zero rank means the player is eligible. Count the
        // strictly farther eligible opponents.
        int unranked = kIneligibleDistanceRank;
        for (j = 0; j < count; j++) {
            distanceRank[j] = unranked;
            Player802E* playerEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
            float playerDistance = playerEntry->distance;
            if (!(playerDistance < minimumEligibleDistance)) {
                distanceRank[j] = distanceRank[j] + 1;
                for (k = 0; k < count; k++) {
                    Player802E* opponentEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)k);
                    float distance = opponentEntry->distance;
                    if (j == k)
                        continue;
                    if (distance < minimumEligibleDistance)
                        continue;
                    if (!(playerDistance < distance))
                        continue;
                    distanceRank[j]++;
                }
            }
        }

        // Starting at -1 makes zero mean exactly one leader. Tied leaders get no
        // sole-leader preference, and no eligible leader leaves this at -1.
        int eligibleLeaderCountMinusOne = -1;
        for (k = 0; k < count; k++) {
            if (distanceRank[k] == 0)
                eligibleLeaderCountMinusOne++;
        }

        for (k = 0; k < count; k++) {
            if (distanceRank[k] == 0 && eligibleLeaderCountMinusOne == 0) {
                soleLeader[k]++;
            }
        }

        // Both ranks start at one and rise for each player ahead, so equal
        // distances share a rank. Only orderedRank counts the tie-break byte.
        // The outer counter is j and the inner one i, which keeps the original's
        // register allocation.
        for (j = 0; j < count; j++) {
            Player802E* rankedPlayerEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
            rankedPlayerEntry->orderedRank = kFirstPlace;

            Player802E* rankedPlayerTieEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
            rankedPlayerTieEntry->tiedRank = kFirstPlace;

            for (i = 0; i < ((CGolfDoc*)DriveGolfDoc())->playerCount; i++) {
                if (j != i) {
                    if (soleLeader[j] < soleLeader[i]) {
                        Player802E* entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                        entry->orderedRank++;

                        entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                        entry->tiedRank++;
                    } else if (soleLeader[j] == soleLeader[i]) {
                        Player802E* opponentEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                        Player802E* rankedPlayerDistanceEntry =
                            GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                        if (rankedPlayerDistanceEntry->distance < opponentEntry->distance) {
                            Player802E* entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                            entry->orderedRank++;

                            entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                            entry->tiedRank++;
                        } else {
                            opponentEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                            rankedPlayerDistanceEntry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                            if (rankedPlayerDistanceEntry->distance == opponentEntry->distance) {
                                Player802E* opponentTieBreak =
                                    GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)i);
                                Player802E* rankedTieBreak =
                                    GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                                unsigned char otherTieBreakOrder = opponentTieBreak->rankTieBreakOrder;
                                unsigned char tieBreakOrder = rankedTieBreak->rankTieBreakOrder;
                                if (tieBreakOrder > otherTieBreakOrder) {
                                    Player802E* entry = GolfPlayer_GetEntryByIndex(DriveGolfDoc(), (unsigned char)j);
                                    entry->orderedRank++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462574", _ctors_u802E43A8_0, DriveSystem_InitFullTurnAngle);
#pragma pop

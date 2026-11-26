// CComThinkShotPos: the rival's list of places it may aim a shot.
//
// Each candidate is a named point with a radius, a base priority, and a cached
// per-player priority adjustment. Candidates live in one array that owns its
// names; special markers are also keyed by priority in a small tree. The class
// declares no virtual functions: its vtable is original game data, so `vtbl` is
// an explicit first member.
#ifndef CLASSES_CCOMTHINKSHOTPOS_H
#define CLASSES_CCOMTHINKSHOTPOS_H

#include "game/types.h"
#include "game/wvector.h"

struct ShotPositionCandidateView;

// The first three words of a candidate name. The engine string stores short
// text in place and long text behind a heap pointer; bit 31 of mode selects the
// heap form, and otherwise the low seven bits of mode are the inline length.
struct ShotPositionCandidateName {
    unsigned int mode; // 0x00
    unsigned int size; // 0x04
    char* data;        // 0x08
};

// A named marker. A base priority of -1 bypasses the inside-radius exclusion
// but still obeys the maximum edge distance.
struct ShotPositionCandidate {
    ShotPositionCandidateName name;  // 0x00
    WVector position;                // 0x0C
    float radius;                    // 0x18
    int basePriority;                // 0x1C
    int playerPriorityAdjustment[4]; // 0x20
    int contextId;                   // 0x30
};
typedef char ShotPositionCandidate_size[(sizeof(ShotPositionCandidate) == 0x34) ? 1 : -1];

// A node of the special-marker tree. The candidate is copied into the node.
struct ShotPositionPriorityNode {
    ShotPositionPriorityNode* left;  // 0x00
    ShotPositionPriorityNode* right; // 0x04
    unsigned int parentAndColor;     // 0x08
    int key;                         // 0x0C
    ShotPositionCandidate candidate; // 0x10
};
typedef char ShotPositionPriorityNode_size[(sizeof(ShotPositionPriorityNode) == 0x44) ? 1 : -1];

// The index BuildCandidateTree fills in. It holds borrowed candidate pointers,
// so its nodes do not own the markers.
struct ShotPositionIndexNode {
    ShotPositionIndexNode* left;      // 0x00
    ShotPositionIndexNode* right;     // 0x04
    unsigned int parentAndColor;      // 0x08
    int key;                          // 0x0C
    ShotPositionCandidate* candidate; // 0x10
};
struct ShotPositionIndex {
    unsigned int count;          // 0x00
    ShotPositionIndexNode* root; // 0x04
};

class CComThinkShotPos {
public:
    // Candidate storage: release the map and the candidates' names.
    int ClearCachedData();

    // Append a marker by copying its name and payload; special markers are keyed by base priority.
    int AddCandidate(ShotPositionCandidateView* value);

    // Priority adjustments.
    void ClearPlayerPriorityAdjustments(int playerIndex);
    int AddContextPriorityBonus(int contextId, int playerIndex);

    // Index the candidates whose markers are close enough to the query position,
    // keyed by base priority plus the current player's adjustment.
    int BuildCandidateTree(ShotPositionIndex* destination, const WVector* queryPosition, float maximumEdgeDistance);

    ~CComThinkShotPos();
    CComThinkShotPos();

    void* vtbl;                             // 0x00
    ShotPositionCandidate* candidates;      // 0x04
    unsigned int candidateCount;            // 0x08
    unsigned int candidateCapacity;         // 0x0C
    unsigned int priorityCount;             // 0x10  size of the special-marker tree
    ShotPositionPriorityNode* priorityRoot; // 0x14
    char pad_018[4];                        // 0x18
    void* priorityFirst;                    // 0x1C  first node, or the root slot when empty
};

#define CCOMTHINKSHOTPOS_OFF(f, o) \
    typedef char CComThinkShotPos_off_##f[((int)&((CComThinkShotPos*)0)->f == (o)) ? 1 : -1]
CCOMTHINKSHOTPOS_OFF(candidates, 0x4);
CCOMTHINKSHOTPOS_OFF(candidateCount, 0x8);
CCOMTHINKSHOTPOS_OFF(candidateCapacity, 0xC);
CCOMTHINKSHOTPOS_OFF(priorityCount, 0x10);
CCOMTHINKSHOTPOS_OFF(priorityRoot, 0x14);
CCOMTHINKSHOTPOS_OFF(priorityFirst, 0x1C);
#undef CCOMTHINKSHOTPOS_OFF
typedef char CComThinkShotPos_size[(sizeof(CComThinkShotPos) == 0x20) ? 1 : -1];

#endif // CLASSES_CCOMTHINKSHOTPOS_H

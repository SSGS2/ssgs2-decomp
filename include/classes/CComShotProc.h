// CComShotProc: the rival's candidate-shot pipeline task.
//
// The pipeline builds a tree of checkpoint candidates around the ball, then
// walks the tree one candidate at a time while the correction chain tunes the
// angle and the power. The task owns the candidate tree and two correction
// chains, one for putts and one for full shots; the rival owner and the chain
// nodes are borrowed.
//
// The class declares no virtual functions: its vtable is original game data, so
// `vtable` is an explicit first member.
#ifndef CLASSES_CCOMSHOTPROC_H
#define CLASSES_CCOMSHOTPROC_H

#include "game/types.h"

class CRival;
class CComPadOnePoint;
class CComShotOnePoint;

// A link in a correction chain. The flag is set when the link is due to run.
struct CorrectionChainLink {
    char pad_00[0x4];
    CorrectionChainLink* next; // 0x04
    int mark;                  // 0x08
};

// One simulated shot and the rules its simulation reported.
struct ShotCandidate {
    float aimAngle;           // 0x00
    float remainingDistance;  // 0x04
    float power;              // 0x08
    float curve;              // 0x0C
    Vec3f position;           // 0x10
    unsigned int rejected;    // 0x1C
    unsigned char groundType; // 0x20
    char pad_21[3];
    unsigned int nearHazard;               // 0x24
    unsigned int firstLandingWithinTarget; // 0x28
    unsigned int finalWithinTarget;        // 0x2C
    unsigned int hasSpecialLanding;        // 0x30
    unsigned int hasInvalidLanding;        // 0x34
    unsigned int holed;                    // 0x38
    unsigned int simulationFrames;         // 0x3C
};

class CComShotProc {
public:
    // Install the vtable, the owner and the two correction chains. Named Init
    // rather than a constructor: callers build the task in raw storage.
    CComShotProc* Init(CRival* owner);
    // Destroy both correction chains and the owned candidate tree.
    ~CComShotProc();

    // Candidate pipeline.
    void RebuildCandidateTree();
    void ApplyCheckpointAndStartCorrection();
    int NextCandidateOrUseFallback();

    // Vtable slot 2: leave the collision system's simulation mode.
    void DisableSimulationMode();

    void* vtable;                               // 0x00
    CorrectionChainLink* currentCorrectionNode; // 0x04
    void* taskState;                            // 0x08
    char helperState[0x4];                      // 0x0C  state handed to the one-point helpers
    float puttRemainingDistance;                // 0x10
    char pad_014[0x14];                         // 0x14
    int puttCandidateRejected;                  // 0x28
    char pad_02C[0x20];                         // 0x2C
    union {
        ShotCandidate bestCandidate; // 0x4C  the best simulated shot so far
        struct {
            char pad_4C[0x1C];
            int shotCandidateRejected; // 0x68  aliases bestCandidate.rejected
            char pad_6C[0x20];
        };
    };
    CComPadOnePoint* padOnePoint;   // 0x8C
    CComShotOnePoint* shotOnePoint; // 0x90
    CRival* owner;                  // 0x94
    void* candidateCount;           // 0x98  tree handle passed to the tree routines
    void* candidateRoot;            // 0x9C
    void* firstCandidate;           // 0xA0
    void* candidateIterator;        // 0xA4
};

#define CCOMSHOTPROC_OFF(f, o) typedef char CComShotProc_off_##f[((int)&((CComShotProc*)0)->f == (o)) ? 1 : -1]
CCOMSHOTPROC_OFF(currentCorrectionNode, 0x4);
CCOMSHOTPROC_OFF(taskState, 0x8);
CCOMSHOTPROC_OFF(puttRemainingDistance, 0x10);
CCOMSHOTPROC_OFF(puttCandidateRejected, 0x28);
CCOMSHOTPROC_OFF(bestCandidate, 0x4C);
CCOMSHOTPROC_OFF(shotCandidateRejected, 0x68);
CCOMSHOTPROC_OFF(padOnePoint, 0x8c);
CCOMSHOTPROC_OFF(shotOnePoint, 0x90);
CCOMSHOTPROC_OFF(owner, 0x94);
CCOMSHOTPROC_OFF(candidateCount, 0x98);
CCOMSHOTPROC_OFF(candidateRoot, 0x9c);
CCOMSHOTPROC_OFF(firstCandidate, 0xa0);
CCOMSHOTPROC_OFF(candidateIterator, 0xa4);
#undef CCOMSHOTPROC_OFF
typedef char CComShotProc_size[(sizeof(CComShotProc) == 0xA8) ? 1 : -1];
typedef char ShotCandidate_size[(sizeof(ShotCandidate) == 0x40) ? 1 : -1];

#endif // CLASSES_CCOMSHOTPROC_H

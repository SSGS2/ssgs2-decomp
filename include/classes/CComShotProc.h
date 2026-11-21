// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMSHOTPROC_H
#define CLASSES_CCOMSHOTPROC_H

#include "game/types.h"

class CComShotProc {
public:
    void* vtable;                        // 0x0: 1 use in CComShotProc_Ctor
    void* currentCorrectionNode;                        // 0x4: 1 use in CComShotProc_Ctor
    void* taskState;                        // 0x8: 1 use in CComShotProc_Ctor
    char pad_00C[0x4];                  // 0xc
    float puttRemainingDistance;                       // 0x10: 1 use in CComShotProc_RebuildCandidateTree
    char pad_014[0x14];                 // 0x14
    int puttCandidateRejected;                         // 0x28: 3 uses in CComShotProc_ApplyCheckpointAndStartCorrection, CComShotProc_RebuildCandidateTree, CComShotProc_NextCandidateOrUseFallback
    char pad_02C[0x3c];                 // 0x2c
    int shotCandidateRejected;                         // 0x68: 2 uses in CComShotProc_RebuildCandidateTree, CComShotProc_NextCandidateOrUseFallback
    char pad_06C[0x20];                 // 0x6c
    void* padOnePoint;                       // 0x8c: 2 uses in CComShotProc_ApplyCheckpointAndStartCorrection, CComShotProc_Ctor
    void* shotOnePoint;                       // 0x90: 2 uses in CComShotProc_ApplyCheckpointAndStartCorrection, CComShotProc_Ctor
    void* owner;                       // 0x94: 21 uses in CComShotProc_NextCandidateOrUseFallback, CComShotProc_Ctor, CComShotProc_RebuildCandidateTree
    void* candidateCount;                       // 0x98: 2 uses in CComShotProc_RebuildCandidateTree, CComShotProc_Ctor
    void* candidateRoot;                       // 0x9c: 3 uses in CComShotProc_RebuildCandidateTree, CComShotProc_Ctor
    void* firstCandidate;                       // 0xa0: 3 uses in CComShotProc_RebuildCandidateTree, CComShotProc_Ctor
    void* candidateIterator;                       // 0xa4: 7 uses in CComShotProc_NextCandidateOrUseFallback, CComShotProc_RebuildCandidateTree

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc, 0x4c, 0x98, 0x9c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMSHOTPROC_OFF(f, o) typedef char CComShotProc_off_##f[((int)&((CComShotProc*)0)->f == (o)) ? 1 : -1]
CCOMSHOTPROC_OFF(currentCorrectionNode, 0x4);
CCOMSHOTPROC_OFF(taskState, 0x8);
CCOMSHOTPROC_OFF(puttRemainingDistance, 0x10);
CCOMSHOTPROC_OFF(puttCandidateRejected, 0x28);
CCOMSHOTPROC_OFF(shotCandidateRejected, 0x68);
CCOMSHOTPROC_OFF(padOnePoint, 0x8c);
CCOMSHOTPROC_OFF(shotOnePoint, 0x90);
CCOMSHOTPROC_OFF(owner, 0x94);
CCOMSHOTPROC_OFF(candidateCount, 0x98);
CCOMSHOTPROC_OFF(candidateRoot, 0x9c);
CCOMSHOTPROC_OFF(firstCandidate, 0xa0);
CCOMSHOTPROC_OFF(candidateIterator, 0xa4);
#undef CCOMSHOTPROC_OFF

#endif  // CLASSES_CCOMSHOTPROC_H

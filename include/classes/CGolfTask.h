// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGOLFTASK_H
#define CLASSES_CGOLFTASK_H

#include "game/types.h"

class CGolfTask {
public:
    void* vtable;                       // 0x0: 1 use in CGolfTask_Ctor
    void* secondaryVtable;              // 0x4: 1 use in CGolfTask_Ctor
    void* tertiaryVtable;               // 0x8: 1 use in CGolfTask_Ctor
    unsigned char unk_C;                // 0xc: 1 use in CGolfTask_Ctor
    char pad_00D[0x39b];                // 0xd
    int treeNodeCount;                        // 0x3a8: 1 use in CGolfTask_Ctor
    void* treeRoot;                      // 0x3ac: 2 uses in CGolfTask_Ctor, CGolfTask_dtor
    void* treeSentinel;                      // 0x3b0: 1 use in CGolfTask_Ctor
    float delayTimer;                      // 0x3b4: 1 use in CGolfTask_Ctor
    int pauseScoreMenu;                        // 0x3b8: 1 use in CGolfTask_Ctor
    int messageActive;                        // 0x3bc: 1 use in CGolfTask_Ctor
    int loadingDraw;                        // 0x3c0: 1 use in CGolfTask_Ctor
    int loadingDrawFinished;                        // 0x3c4: 1 use in CGolfTask_Ctor
    char pad_3C8[0x18];                 // 0x3c8
    int variantPreloadFile;                        // 0x3e0: 1 use in CGolfTask_Ctor
    int inGameTexturePack1;                        // 0x3e4: 1 use in CGolfTask_Ctor
    int inGameTexturePack2;                        // 0x3e8: 1 use in CGolfTask_Ctor
    int tutorialTexturePack;                        // 0x3ec: 1 use in CGolfTask_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x3a8, 0x3ac, 0x3c8, 0x3e0, 0x3e4, 0x3e8, 0x3ec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGOLFTASK_OFF(f, o) typedef char CGolfTask_off_##f[((int)&((CGolfTask*)0)->f == (o)) ? 1 : -1]
CGOLFTASK_OFF(secondaryVtable, 0x4);
CGOLFTASK_OFF(tertiaryVtable, 0x8);
CGOLFTASK_OFF(unk_C, 0xc);
CGOLFTASK_OFF(treeNodeCount, 0x3a8);
CGOLFTASK_OFF(treeRoot, 0x3ac);
CGOLFTASK_OFF(treeSentinel, 0x3b0);
CGOLFTASK_OFF(delayTimer, 0x3b4);
CGOLFTASK_OFF(pauseScoreMenu, 0x3b8);
CGOLFTASK_OFF(messageActive, 0x3bc);
CGOLFTASK_OFF(loadingDraw, 0x3c0);
CGOLFTASK_OFF(loadingDrawFinished, 0x3c4);
CGOLFTASK_OFF(variantPreloadFile, 0x3e0);
CGOLFTASK_OFF(inGameTexturePack1, 0x3e4);
CGOLFTASK_OFF(inGameTexturePack2, 0x3e8);
CGOLFTASK_OFF(tutorialTexturePack, 0x3ec);
#undef CGOLFTASK_OFF

#endif  // CLASSES_CGOLFTASK_H

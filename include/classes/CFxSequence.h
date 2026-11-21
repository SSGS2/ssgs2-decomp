// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXSEQUENCE_H
#define CLASSES_CFXSEQUENCE_H

#include "game/types.h"
#include "game/wvector.h"

class CFxSequence {
public:
    void* vtbl;                        // 0x0: __vt__11CFxSequence
    char unk_4;                         // 0x4: 1 use in CFxSequence_ctor
    char pad_005[0xf];                  // 0x5
    unsigned int attachment;                // 0x14: 1 use in CFxSequence_ctor
    float attachmentPositionX;                       // 0x18: 1 use in CFxSequence_ctor
    float attachmentPositionY;                       // 0x1c: 1 use in CFxSequence_ctor
    unsigned int sprayCount;                // 0x20: 1 use in CFxSequence_ctor
    unsigned int sprayTreeRoot;                // 0x24: 2 uses in CFxSequence_ctor, CFxSequence_dtor
    void* firstSprayNode;                       // 0x28: 1 use in CFxSequence_ctor
    char pad_02C[0x50];                 // 0x2c
    void* sequenceOwner;                // 0x7c: creates and registers this sequence's sprays
    char playbackEnabled;                        // 0x80: 1 use in CFxSequence_ctor
    char pad_081[0x3];                  // 0x81
    WVector worldPosition;             // 0x84: CFxTomahawk_AdvancePhase / CFxSpike_UpdateAndAdvanceState copy CGolfBall position (fx_shot_effects_tomahawk_spike.cpp)
    char pad_090[0xc];                  // 0x90
    char deleteWhenRemovedFromBox;                        // 0x9c: 1 use in CFxSequence_ctor
    char pad_09D[0x3];                  // 0x9d
    float elapsedTime;                  // 0xa0: script timeline advanced by CFxSequence_UpdateTimeline
    float resumeDelay;                       // 0xa4: 1 use in CFxSequence_ctor
    char pad_0A8[0x10];                 // 0xa8
    char failedCommandText;                        // 0xb8: 1 use in CFxSequence_ctor
    char pad_0B9[0x103];                // 0xb9
    unsigned int currentSpray;          // 0x1bc: spray receiving parsed actions and tracks
    unsigned int commandCount;          // 0x1c0: number of parsed script command nodes
    char pad_1C4[0x4];                  // 0x1c4
    void* firstCommandNode;             // 0x1c8: sentinel+4 (first command node, or the sentinel when empty); command traversal starts here
    void* nextCommand;                  // 0x1cc: next script command to execute
    void* effectBox;                      // 0x1d0: 4 uses in CFxSequence_dtor, CFxSequence_ctor
    char bypassResourceCache;                       // 0x1d4: 1 use in CFxSequence_ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x20, 0x24, 0x2c, 0x3c, 0x4c, 0x84, 0x90, 0x1c0, 0x1c4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXSEQUENCE_OFF(f, o) typedef char CFxSequence_off_##f[((int)&((CFxSequence*)0)->f == (o)) ? 1 : -1]
CFXSEQUENCE_OFF(vtbl, 0x0);
CFXSEQUENCE_OFF(unk_4, 0x4);
CFXSEQUENCE_OFF(attachment, 0x14);
CFXSEQUENCE_OFF(attachmentPositionX, 0x18);
CFXSEQUENCE_OFF(attachmentPositionY, 0x1c);
CFXSEQUENCE_OFF(sprayCount, 0x20);
CFXSEQUENCE_OFF(sprayTreeRoot, 0x24);
CFXSEQUENCE_OFF(firstSprayNode, 0x28);
CFXSEQUENCE_OFF(sequenceOwner, 0x7c);
CFXSEQUENCE_OFF(playbackEnabled, 0x80);
CFXSEQUENCE_OFF(worldPosition, 0x84);
CFXSEQUENCE_OFF(deleteWhenRemovedFromBox, 0x9c);
CFXSEQUENCE_OFF(elapsedTime, 0xa0);
CFXSEQUENCE_OFF(resumeDelay, 0xa4);
CFXSEQUENCE_OFF(failedCommandText, 0xb8);
CFXSEQUENCE_OFF(currentSpray, 0x1bc);
CFXSEQUENCE_OFF(commandCount, 0x1c0);
CFXSEQUENCE_OFF(firstCommandNode, 0x1c8);
CFXSEQUENCE_OFF(nextCommand, 0x1cc);
CFXSEQUENCE_OFF(effectBox, 0x1d0);
CFXSEQUENCE_OFF(bypassResourceCache, 0x1d4);
#undef CFXSEQUENCE_OFF

#endif  // CLASSES_CFXSEQUENCE_H

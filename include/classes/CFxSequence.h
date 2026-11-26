// CFxSequence: one playing effect sequence.
//
// It owns the parsed command list, the timeline cursor, the spray currently
// receiving commands, and the attachment/wind state. The vtable is an explicit
// member because it still comes from the original game data.
#ifndef CLASSES_CFXSEQUENCE_H
#define CLASSES_CFXSEQUENCE_H

#include "game/types.h"
#include "game/wvector.h"

class CFxSequence {
public:
    // Construction and script loading.
    CFxSequence(void* owner, unsigned int isCachedTemplate);
    int LoadByName(const char* name, unsigned int bypassCache);

    // Script parsing and execution.
    void ParseEntryLine(const char* input, unsigned int sourceLine);
    int ExecuteScriptCommand(signed char* input, unsigned char* result);
    void RandomizeAndSortCommandTimes();

    // Timeline.
    int UpdateTimeline(float dt, unsigned char* flag);

    void ClearCommands();

    // Sprays created by this sequence, kept in a tree keyed by spray address.
    void UnregisterSpray(unsigned int key);
    void SetWindVector(void* wind);
    void InvokeSprayLifecycleHooks(const void* arg);

    ~CFxSequence();

    void* vtbl;                 // 0x0: __vt__11CFxSequence
    char clearOnPartialClear;   // 0x4
    char pad_005[0x3];          // 0x5
    WVector wind;               // 0x8: wind applied to the sprays this sequence creates
    unsigned int attachment;    // 0x14
    float attachmentPositionX;  // 0x18
    float attachmentPositionY;  // 0x1c
    unsigned int sprayCount;    // 0x20
    unsigned int sprayTreeRoot; // 0x24
    void* firstSprayNode;       // 0x28
    char pad_02C[0x20];         // 0x2c
    float cameraMatrix[12];     // 0x4c: camera basis used when a script command rotates its vectors
    void* sequenceOwner;        // 0x7c: creates and registers this sequence's sprays
    char playbackEnabled;       // 0x80
    char pad_081[0x3];          // 0x81
    WVector
        worldPosition; // 0x84: CFxTomahawk_AdvancePhase / CFxSpike_UpdateAndAdvanceState copy CGolfBall position (fx_shot_effects_tomahawk_spike.cpp)
    char pad_090[0xc]; // 0x90
    char deleteWhenRemovedFromBox; // 0x9c
    char pad_09D[0x3];             // 0x9d
    float elapsedTime;             // 0xa0: script timeline advanced by CFxSequence_UpdateTimeline
    float resumeDelay;             // 0xa4
    char pad_0A8[0x10];            // 0xa8
    char failedCommandText;        // 0xb8
    char pad_0B9[0x103];           // 0xb9
    unsigned int currentSpray;     // 0x1bc: spray receiving parsed actions and tracks
    unsigned int commandCount;     // 0x1c0: number of parsed script command nodes
    char pad_1C4[0x4];             // 0x1c4
    void*
        firstCommandNode; // 0x1c8: sentinel+4 (first command node, or the sentinel when empty); command traversal starts here
    void* nextCommand;        // 0x1cc: next script command to execute
    void* effectBox;          // 0x1d0
    char bypassResourceCache; // 0x1d4
};

#define CFXSEQUENCE_OFF(f, o) typedef char CFxSequence_off_##f[((int)&((CFxSequence*)0)->f == (o)) ? 1 : -1]
CFXSEQUENCE_OFF(vtbl, 0x0);
CFXSEQUENCE_OFF(clearOnPartialClear, 0x4);
CFXSEQUENCE_OFF(wind, 0x8);
CFXSEQUENCE_OFF(attachment, 0x14);
CFXSEQUENCE_OFF(cameraMatrix, 0x4c);
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
typedef char CFxSequence_size[(sizeof(CFxSequence) == 0x1d8) ? 1 : -1];

#endif // CLASSES_CFXSEQUENCE_H

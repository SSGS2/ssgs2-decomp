#ifndef CLASSES_CB2DPLAYERSIMPLE_H
#define CLASSES_CB2DPLAYERSIMPLE_H

#include "game/types.h"

class CB2dPlayerSimple {
public:
    int bpObject;                             // 0x0
    int state;                                // 0x4
    unsigned int bpFileNameStorageFlags;      // 0x8
    int bpFileNameLength;                     // 0xc
    void* bpFileNameHeap;                     // 0x10
    unsigned int initialScriptStorageFlags;   // 0x14
    int initialScriptLength;                  // 0x18
    void* initialScriptHeap;                  // 0x1c
    unsigned int requestedScriptStorageFlags; // 0x20
    int requestedScriptLength;                // 0x24
    void* requestedScriptHeap;                // 0x28
    unsigned int repeatScriptStorageFlags;    // 0x2c
    int repeatScriptLength;                   // 0x30
    void* repeatScriptHeap;                   // 0x34
    short scriptRepeatsRemaining;             // 0x38: CB2dPlayerSimple_ConfigurePlayback copies parameter d
    char pad_03A[2];
    int pendingScript; // 0x3C: CB2dPlayerSimple_StartPendingScript gates the queued script, CB2dPlayerHandle_RequestPendingScript sets it
    void* resolvedNodes;              // 0x40
    int resolvedNodeCount;            // 0x44
    int resolvedNodeCapacity;         // 0x48
    void* pendingNodeNames;           // 0x4c
    int pendingNodeNameCount;         // 0x50
    int pendingNodeNameCapacity;      // 0x54
    void* pendingItemIds;             // 0x58
    int pendingItemIdCount;           // 0x5c
    int pendingItemIdCapacity;        // 0x60
    void* pendingPulseNodeNames;      // 0x64
    int pendingPulseNodeNameCount;    // 0x68
    int pendingPulseNodeNameCapacity; // 0x6c
    unsigned char pulseStep;          // 0x70
    char pad_071[0x3];                // 0x71
    int pulsePhase; // 0x74: B2dPlayer_UpdateAndDraw adds pulseStep, wraps above 255 by 510, applies abs to resolved nodes
    void* vtable;   // 0x78

    void SetSpritesEnabled(void* enabledValue);
    static void ResetOpacityPulse(void* object);
    void Init();
    static int StartPendingScript(void* object);
    static void UnloadAndClearPendingState(void* object);
    static void ConfigurePlayback(void* object, const char* bpFileName, const char* initialScript,
                                  int requestedScriptAddress, int repeatScriptAddress, int repeatCount);
    static void SetPendingPulseNodeNames(void* object, const char** nodeNames, int nodeCount, int opacityDivisor);
    static void SetPendingNodeNames(void* object, const char** nodeNames, int nodeCount);
    static void SetPendingItemIds(void* object, const float* itemIds, unsigned int itemCount);
    static void ResolvePendingNodes(void* object);
    static void ResolvePendingPulseNodes(void* object);

    ~CB2dPlayerSimple();
};

#define CB2DPLAYERSIMPLE_OFF(f, o) \
    typedef char CB2dPlayerSimple_off_##f[((int)&((CB2dPlayerSimple*)0)->f == (o)) ? 1 : -1]
CB2DPLAYERSIMPLE_OFF(state, 0x4);
CB2DPLAYERSIMPLE_OFF(bpFileNameStorageFlags, 0x8);
CB2DPLAYERSIMPLE_OFF(bpFileNameLength, 0xc);
CB2DPLAYERSIMPLE_OFF(bpFileNameHeap, 0x10);
CB2DPLAYERSIMPLE_OFF(initialScriptStorageFlags, 0x14);
CB2DPLAYERSIMPLE_OFF(initialScriptLength, 0x18);
CB2DPLAYERSIMPLE_OFF(initialScriptHeap, 0x1c);
CB2DPLAYERSIMPLE_OFF(requestedScriptStorageFlags, 0x20);
CB2DPLAYERSIMPLE_OFF(requestedScriptLength, 0x24);
CB2DPLAYERSIMPLE_OFF(requestedScriptHeap, 0x28);
CB2DPLAYERSIMPLE_OFF(repeatScriptStorageFlags, 0x2c);
CB2DPLAYERSIMPLE_OFF(repeatScriptLength, 0x30);
CB2DPLAYERSIMPLE_OFF(repeatScriptHeap, 0x34);
CB2DPLAYERSIMPLE_OFF(resolvedNodes, 0x40);
CB2DPLAYERSIMPLE_OFF(resolvedNodeCount, 0x44);
CB2DPLAYERSIMPLE_OFF(resolvedNodeCapacity, 0x48);
CB2DPLAYERSIMPLE_OFF(pendingNodeNames, 0x4c);
CB2DPLAYERSIMPLE_OFF(pendingNodeNameCount, 0x50);
CB2DPLAYERSIMPLE_OFF(pendingNodeNameCapacity, 0x54);
CB2DPLAYERSIMPLE_OFF(pendingItemIds, 0x58);
CB2DPLAYERSIMPLE_OFF(pendingItemIdCount, 0x5c);
CB2DPLAYERSIMPLE_OFF(pendingItemIdCapacity, 0x60);
CB2DPLAYERSIMPLE_OFF(pendingPulseNodeNames, 0x64);
CB2DPLAYERSIMPLE_OFF(pendingPulseNodeNameCount, 0x68);
CB2DPLAYERSIMPLE_OFF(pendingPulseNodeNameCapacity, 0x6c);
CB2DPLAYERSIMPLE_OFF(pulseStep, 0x70);
CB2DPLAYERSIMPLE_OFF(pulsePhase, 0x74);
CB2DPLAYERSIMPLE_OFF(vtable, 0x78);
CB2DPLAYERSIMPLE_OFF(bpObject, 0x0);
CB2DPLAYERSIMPLE_OFF(scriptRepeatsRemaining, 0x38);
CB2DPLAYERSIMPLE_OFF(pendingScript, 0x3C);
#undef CB2DPLAYERSIMPLE_OFF

#endif // CLASSES_CB2DPLAYERSIMPLE_H

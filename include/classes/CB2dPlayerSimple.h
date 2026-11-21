// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CB2DPLAYERSIMPLE_H
#define CLASSES_CB2DPLAYERSIMPLE_H

#include "game/types.h"

class CB2dPlayerSimple {
public:
    int bpObject;                          // 0x0: 1 use in CB2dPlayerSimple_Init
    int state;                          // 0x4: 1 use in CB2dPlayerSimple_Init
    unsigned int bpFileNameStorageFlags;                 // 0x8: 5 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init (CONFLICT sizes [1, 4])
    int bpFileNameLength;                          // 0xc: 2 uses in CB2dPlayerSimple_Init, CB2dPlayerSimple_dtor
    void* bpFileNameHeap;                       // 0x10: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    unsigned int initialScriptStorageFlags;                // 0x14: 5 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init (CONFLICT sizes [1, 4])
    int initialScriptLength;                         // 0x18: 2 uses in CB2dPlayerSimple_Init, CB2dPlayerSimple_dtor
    void* initialScriptHeap;                       // 0x1c: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    unsigned int requestedScriptStorageFlags;                // 0x20: 5 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init (CONFLICT sizes [1, 4])
    int requestedScriptLength;                         // 0x24: 2 uses in CB2dPlayerSimple_Init, CB2dPlayerSimple_dtor
    void* requestedScriptHeap;                       // 0x28: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    unsigned int repeatScriptStorageFlags;                // 0x2c: 5 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init (CONFLICT sizes [1, 4])
    int repeatScriptLength;                         // 0x30: 2 uses in CB2dPlayerSimple_Init, CB2dPlayerSimple_dtor
    void* repeatScriptHeap;                       // 0x34: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    short scriptRepeatsRemaining; // 0x38: CB2dPlayerSimple_ConfigurePlayback copies parameter d
    char pad_03A[2];
    int pendingScript; // 0x3C: CB2dPlayerSimple_StartPendingScript gates the queued script, CB2dPlayerHandle_RequestPendingScript sets it
    void* resolvedNodes;                       // 0x40: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int resolvedNodeCount;                         // 0x44: 2 uses in CB2dPlayerSimple_Init, CB2dPlayerSimple_dtor
    int resolvedNodeCapacity;                         // 0x48: 1 use in CB2dPlayerSimple_Init
    void* pendingNodeNames;                       // 0x4c: 6 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int pendingNodeNameCount;                         // 0x50: 7 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int pendingNodeNameCapacity;                         // 0x54: 1 use in CB2dPlayerSimple_Init
    void* pendingItemIds;                       // 0x58: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int pendingItemIdCount;                         // 0x5c: 3 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int pendingItemIdCapacity;                         // 0x60: 1 use in CB2dPlayerSimple_Init
    void* pendingPulseNodeNames;                       // 0x64: 5 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int pendingPulseNodeNameCount;                         // 0x68: 5 uses in CB2dPlayerSimple_dtor, CB2dPlayerSimple_Init
    int pendingPulseNodeNameCapacity;                         // 0x6c: 1 use in CB2dPlayerSimple_Init
    unsigned char pulseStep;               // 0x70: 1 use in CB2dPlayerSimple_Init
    char pad_071[0x3];                  // 0x71
    int pulsePhase; // 0x74: B2dPlayer_UpdateAndDraw adds pulseStep, wraps above 255 by 510, applies abs to resolved nodes
    void* vtable;                       // 0x78: 2 uses in CB2dPlayerSimple_Init, CB2dPlayerSimple_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CB2DPLAYERSIMPLE_OFF(f, o) typedef char CB2dPlayerSimple_off_##f[((int)&((CB2dPlayerSimple*)0)->f == (o)) ? 1 : -1]
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

#endif  // CLASSES_CB2DPLAYERSIMPLE_H

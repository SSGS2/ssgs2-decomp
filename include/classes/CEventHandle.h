// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTHANDLE_H
#define CLASSES_CEVENTHANDLE_H

#include "game/types.h"

class CEventHandle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];
    unsigned int unk_8;                 // 0x8: 1 use in EventScript_RunBlocking
    char pad_00C[0x10];
    int storyState;                         // 0x1c: 9 uses in CStoryConte_OnProcess
    unsigned int storyIndex;                // 0x20: 1 use in CStoryConte_OnProcess
    char pad_024[0x8];
    void* eventPathData;                       // 0x2c: 1 use in CStoryConte_ProcessPlayerEventScripts
    void* eventLoad;                       // 0x30: 2 uses in CStoryConte_OnProcess
    void* eventHandle;                       // 0x34: 8 uses in CStoryConte_OnProcess
    void* eventDataLoad;                       // 0x38: 2 uses in CStoryConte_OnProcess
    char pad_03C[0xC];
    unsigned char skipFadeUpdateCount;               // 0x48: 5 uses in CStoryConte_OnProcess
    char pad_049[0x53];
    void* storySkipCaptionResource;                       // 0x9c: 1 use in CStoryConte_OnProcess
    char pad_0A0[0x368];
    unsigned int nameFlags; // 0x408: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int nameLength; // 0x40c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void * nameData; // 0x410: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void * argument; // 0x414: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void * tree; // 0x418: EventScript_RunBlocking (0x8023F8DC) accesses this member
    char pad_41C[0x4];
    void* ownedCommandHandler0;                      // 0x420: 2 uses in CEventHandle_dtor
    void* pendingModelEvent;                      // 0x424: 2 uses in CEventHandle_dtor
    void* pendingEffectEvent;                      // 0x428: 2 uses in CEventHandle_dtor
    void* pendingMessageRequest;                      // 0x42c: 2 uses in CEventHandle_dtor
    void* pendingSoundEvent;                      // 0x430: 2 uses in CEventHandle_dtor
    unsigned int unk_434; // 0x434: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_438; // 0x438: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_43C; // 0x43c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_440; // 0x440: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_444; // 0x444: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_448; // 0x448: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_44C; // 0x44c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_450; // 0x450: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_454; // 0x454: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_458; // 0x458: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_45C; // 0x45c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_460; // 0x460: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_464; // 0x464: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_468; // 0x468: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_46C; // 0x46c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_470; // 0x470: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_474; // 0x474: EventScript_RunBlocking (0x8023F8DC) accesses this member
    char pad_478[0x14];
    void* pendingResource;                      // 0x48c: 2 uses in CEventHandle_dtor
    char pad_490[0xC];
    unsigned int unk_49C; // 0x49c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_4A0; // 0x4a0: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void* eventVtable;                      // 0x4a4: 1 use in CEventHandle_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTHANDLE_OFF(f, o) typedef char CEventHandle_off_##f[((int)&((CEventHandle*)0)->f == (o)) ? 1 : -1]
CEVENTHANDLE_OFF(vtbl, 0x0);
CEVENTHANDLE_OFF(unk_8, 0x8);
CEVENTHANDLE_OFF(storyState, 0x1c);
CEVENTHANDLE_OFF(storyIndex, 0x20);
CEVENTHANDLE_OFF(eventPathData, 0x2c);
CEVENTHANDLE_OFF(eventLoad, 0x30);
CEVENTHANDLE_OFF(eventHandle, 0x34);
CEVENTHANDLE_OFF(eventDataLoad, 0x38);
CEVENTHANDLE_OFF(skipFadeUpdateCount, 0x48);
CEVENTHANDLE_OFF(storySkipCaptionResource, 0x9c);
CEVENTHANDLE_OFF(nameFlags, 0x408);
CEVENTHANDLE_OFF(nameLength, 0x40c);
CEVENTHANDLE_OFF(nameData, 0x410);
CEVENTHANDLE_OFF(argument, 0x414);
CEVENTHANDLE_OFF(tree, 0x418);
CEVENTHANDLE_OFF(ownedCommandHandler0, 0x420);
CEVENTHANDLE_OFF(pendingModelEvent, 0x424);
CEVENTHANDLE_OFF(pendingEffectEvent, 0x428);
CEVENTHANDLE_OFF(pendingMessageRequest, 0x42c);
CEVENTHANDLE_OFF(pendingSoundEvent, 0x430);
CEVENTHANDLE_OFF(unk_434, 0x434);
CEVENTHANDLE_OFF(unk_438, 0x438);
CEVENTHANDLE_OFF(unk_43C, 0x43c);
CEVENTHANDLE_OFF(unk_440, 0x440);
CEVENTHANDLE_OFF(unk_444, 0x444);
CEVENTHANDLE_OFF(unk_448, 0x448);
CEVENTHANDLE_OFF(unk_44C, 0x44c);
CEVENTHANDLE_OFF(unk_450, 0x450);
CEVENTHANDLE_OFF(unk_454, 0x454);
CEVENTHANDLE_OFF(unk_458, 0x458);
CEVENTHANDLE_OFF(unk_45C, 0x45c);
CEVENTHANDLE_OFF(unk_460, 0x460);
CEVENTHANDLE_OFF(unk_464, 0x464);
CEVENTHANDLE_OFF(unk_468, 0x468);
CEVENTHANDLE_OFF(unk_46C, 0x46c);
CEVENTHANDLE_OFF(unk_470, 0x470);
CEVENTHANDLE_OFF(unk_474, 0x474);
CEVENTHANDLE_OFF(pendingResource, 0x48c);
CEVENTHANDLE_OFF(unk_49C, 0x49c);
CEVENTHANDLE_OFF(unk_4A0, 0x4a0);
CEVENTHANDLE_OFF(eventVtable, 0x4a4);
#undef CEVENTHANDLE_OFF

#endif  // CLASSES_CEVENTHANDLE_H

#ifndef CLASSES_CEVENTHANDLE_H
#define CLASSES_CEVENTHANDLE_H

#include "game/types.h"

struct EventObjectNodeView;
class CEventHandle {
public:
    // A pointer vector owned by the handle: allocation, live count, capacity.
    struct PointerVector {
        void** begin;          // 0x0
        unsigned int count;    // 0x4
        unsigned int capacity; // 0x8
    };
    // An ordered tree: element count, root node, and the end sentinel that
    // points at the root link.
    struct Tree {
        unsigned int count; // 0x0
        void* root;         // 0x4
        void** end;         // 0x8
    };

    // Copy the interpreter name and install the borrowed script addresses.
    CEventHandle(const char* text, void* scriptData, unsigned int entryAddress);
    ~CEventHandle();

    void* vtbl; // 0x0: original vtable pointer
    char pad_004[0x4];
    unsigned int unk_8; // 0x8
    char pad_00C[0x10];
    int storyState;          // 0x1c
    unsigned int storyIndex; // 0x20
    char pad_024[0x8];
    void* eventPathData; // 0x2c
    void* eventLoad;     // 0x30
    void* eventHandle;   // 0x34
    void* eventDataLoad; // 0x38
    char pad_03C[0xC];
    unsigned char skipFadeUpdateCount; // 0x48
    char pad_049[0x53];
    void* storySkipCaptionResource; // 0x9c
    char pad_0A0[0x368];
    unsigned int nameFlags;  // 0x408: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int nameLength; // 0x40c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void* nameData;          // 0x410: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void* argument;          // 0x414: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void* tree;              // 0x418: EventScript_RunBlocking (0x8023F8DC) accesses this member
    char pad_41C[0x4];
    void* ownedCommandHandler0;   // 0x420
    void* pendingModelEvent;      // 0x424
    void* pendingEffectEvent;     // 0x428
    void* pendingMessageRequest;  // 0x42c
    void* pendingSoundEvent;      // 0x430
    PointerVector objectLists[5]; // 0x434: cameras, models, effects, messages, sounds
    Tree objectTree;              // 0x470
    char pad_47C[0x10];           // 0x47c: embedded effect parser
    void* pendingResource;        // 0x48c
    Tree resourceTree;            // 0x490
    unsigned int unk_49C;         // 0x49c: EventScript_RunBlocking (0x8023F8DC) accesses this member
    unsigned int unk_4A0;         // 0x4a0: EventScript_RunBlocking (0x8023F8DC) accesses this member
    void* eventVtable;            // 0x4a4

    void DestroyTreeRangeObjects(EventObjectNodeView** cursor, EventObjectNodeView** limit);
    void ClearTree();
};

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
CEVENTHANDLE_OFF(objectLists, 0x434);
CEVENTHANDLE_OFF(objectTree, 0x470);
CEVENTHANDLE_OFF(pendingResource, 0x48c);
CEVENTHANDLE_OFF(resourceTree, 0x490);
CEVENTHANDLE_OFF(unk_49C, 0x49c);
CEVENTHANDLE_OFF(unk_4A0, 0x4a0);
CEVENTHANDLE_OFF(eventVtable, 0x4a4);
#undef CEVENTHANDLE_OFF
typedef char CEventHandle_size[(sizeof(CEventHandle) == 0x4A8) ? 1 : -1];

#endif // CLASSES_CEVENTHANDLE_H

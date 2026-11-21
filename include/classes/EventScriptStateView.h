// EventScriptStateView: layout view; methods in event_conte_script_interpreter.cpp (EventScript_RunAndSweep clears elapsedCommandTicks; command handlers advance cursor; CEventConteProc_ProcessIsTourClearCommand counts conditions).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SCRIPTSTATE80092804_H
#define CLASSES_SCRIPTSTATE80092804_H

class EventScriptStateView {
public:
    char pad_000[0x2];
    unsigned short flags; // 0x2
    char pad_004[0x2];
    unsigned char conditionCount; // 0x6
    unsigned char trueConditionCount; // 0x7
    void* slots[256]; // 0x8: CEventConteProc_ProcessBindFlagVariableCommand/CEventConteProc_ProcessBindParameterVariableCommand store byte-indexed object slots
    unsigned int prefixFlags; // 0x408
    char pad_40C[0x4];
    char* prefixHeap; // 0x410
    char pad_414[0x4];
    unsigned char* cursor; // 0x418
    unsigned int elapsedCommandTicks; // 0x41C
    char pad_420[0x58];
    void* objectTreeFirst; // 0x478: first node, or &root at +0x474 when empty.
    char pad_47C[0x10];
    void* pendingObject; // 0x48C
    char pad_490[0x8];
    void* resourceTreeFirst; // 0x498: first node, or &root at +0x494 when empty.
    unsigned int faceScriptTableBase; // 0x49C: borrowed facial-data buffer address.
    void* manager; // 0x4A0
    void** eventHandleVtable; // 0x4A4
};

#define OFF(f, o) typedef char ScriptState80092804_off_##f[((int)&((EventScriptStateView*)0)->f == (o)) ? 1 : -1]
OFF(flags, 0x2);
OFF(conditionCount, 0x6);
OFF(trueConditionCount, 0x7);
OFF(slots, 0x8);
OFF(prefixFlags, 0x408);
OFF(prefixHeap, 0x410);
OFF(cursor, 0x418);
OFF(elapsedCommandTicks, 0x41C);
OFF(objectTreeFirst, 0x478);
OFF(pendingObject, 0x48C);
OFF(resourceTreeFirst, 0x498);
OFF(faceScriptTableBase, 0x49C);
OFF(manager, 0x4A0);
OFF(eventHandleVtable, 0x4A4);
#undef OFF

#define AsScriptState80092804(p) ((EventScriptStateView*)(p))

#endif

// EventScriptByteTreePair: layout view; methods in event_conte_script_interpreter.cpp (ScriptByteMap_GetOrInsertValue and ScriptSharedFiles_ResetRequiredFilesAndHandlers address the key/value pair at node +0xC).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SCRIPTBYTETREEPAIR80092804_H
#define CLASSES_SCRIPTBYTETREEPAIR80092804_H

class EventScriptByteTreePair {
public:
    unsigned char key; // 0x0
    char pad_001[0x3];
    unsigned int value; // 0x4
};

#define OFF(f, o) typedef char ScriptByteTreePair80092804_off_##f[((int)&((EventScriptByteTreePair*)0)->f == (o)) ? 1 : -1]
OFF(key, 0x0);
OFF(value, 0x4);
#undef OFF

#define AsScriptByteTreePair80092804(p) ((EventScriptByteTreePair*)(p))

#endif

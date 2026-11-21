// EventScriptByteTreeNode: layout view; methods in event_conte_script_interpreter.cpp (EventFxMap_FindInsertionPosition compares key and walks left/right; EventFxMap_InsertNode inserts a 0x14-byte node).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SCRIPTBYTETREENODE80092804_H
#define CLASSES_SCRIPTBYTETREENODE80092804_H

class EventScriptByteTreeNode {
public:
    void* left; // 0x0
    void* right; // 0x4
    void* parent; // 0x8
    unsigned char key; // 0xC
    char pad_00D[0x3];
    unsigned int value; // 0x10
};

#define OFF(f, o) typedef char ScriptByteTreeNode80092804_off_##f[((int)&((EventScriptByteTreeNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parent, 0x8);
OFF(key, 0xC);
OFF(value, 0x10);
#undef OFF

#define AsScriptByteTreeNode80092804(p) ((EventScriptByteTreeNode*)(p))

#endif

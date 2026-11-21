// ScriptOwnedBuffer80092804: layout view; methods in event_conte_script_interpreter.cpp (SharedFilePointerVector_Destroy zeros count and frees data; ScriptOwnedBuffer_dtor clears count).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SCRIPTOWNEDBUFFER80092804_H
#define CLASSES_SCRIPTOWNEDBUFFER80092804_H

class ScriptOwnedBuffer80092804 {
public:
    void* data; // 0x0
    int count; // 0x4
};

#define OFF(f, o) typedef char ScriptOwnedBuffer80092804_off_##f[((int)&((ScriptOwnedBuffer80092804*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x0);
OFF(count, 0x4);
#undef OFF

#define AsScriptOwnedBuffer80092804(p) ((ScriptOwnedBuffer80092804*)(p))

#endif

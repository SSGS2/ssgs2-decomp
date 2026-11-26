// FxInterpreter view: GetInstance (0x8010C3DC) / dtor (0x8010C454) in fx_box_interpreter_resources.cpp.
#ifndef CLASSES_FXINTERPRETER_H
#define CLASSES_FXINTERPRETER_H

struct FxInterpreterLinks {
    void* previous;
    void* next;
};
struct FxInterpreterEntry {
    FxInterpreterLinks links;
    char pad_008[4];
    int kind;    // FxInterpreter_dtor: 2 calls CFx::RemoveSpray; 1 calls CFx::RemoveSequence.
    void* value; // Passed to the selected release function.
};
struct FxInterpreter {
    void* vtbl;
    unsigned int entryCount;        // FxInterpreter_dtor tests and decrements after freeing each entry.
    FxInterpreterLinks sentinel;    // FxInterpreter_GetInstance makes both links point here.
    unsigned int nextAttachmentKey; // FxInterpreter_GetInstance initializes to zero.
};

#define OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
OFF(FxInterpreterLinks, previous, 0x0);
OFF(FxInterpreterLinks, next, 0x4);
OFF(FxInterpreterEntry, links, 0x0);
OFF(FxInterpreterEntry, kind, 0xC);
OFF(FxInterpreterEntry, value, 0x10);
OFF(FxInterpreter, vtbl, 0x0);
OFF(FxInterpreter, entryCount, 0x4);
OFF(FxInterpreter, sentinel, 0x8);
OFF(FxInterpreter, nextAttachmentKey, 0x10);
#undef OFF

#endif

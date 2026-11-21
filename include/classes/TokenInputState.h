// Common input-state view of cTokenV and cTokenS, whose init/getters live in token_parser_and_bubble.cpp.
// No inheritance, constructors, or emitted code.
#ifndef CLASSES_TOKENINPUTSTATE_H
#define CLASSES_TOKENINPUTSTATE_H

struct TokenInputState {
    void* vtable;
    int cursor; // 0x04: cTokenV_SetInputBuffer (0x800D1E78) resets; cToken_ReadUntilDelimiter advances Buffer::used
    int inputCount; // 0x08: same setter rejects count < 1, then stores count
    char pad_0C[8];
    void* inputBuffer; // 0x14: setter stores value; cTokenV/cTokenS_vfunc_1 return it
};

#define OFF(f, o) typedef char TokenInputState_off_##f[((int)&((TokenInputState*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x00);
OFF(cursor, 0x04);
OFF(inputCount, 0x08);
OFF(inputBuffer, 0x14);
#undef OFF
#define AsTokenInputState(p) ((TokenInputState*)(p))
#endif

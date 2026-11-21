// DirectInputMouseState: layout view; directinput_mouse_keyboard.cpp: DirectInputMouse_PollPosition computes position deltas and records OS_GetTimeMilliseconds; ctor initializes and vfuncs query axes
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_DIRECTINPUTMOUSESTATE_H
#define CLASSES_DIRECTINPUTMOUSESTATE_H

struct DirectInputMouseState {
    char pad_000[0x4];
    int registry; // 0x4
    void* inputVtable; // 0x8
    unsigned char enabled; // 0xC
    signed char changed; // 0xD
    char pad_00E[0x2];
    unsigned int lastChangeTime; // 0x10
    int button0QueryState; // 0x14
    int button1QueryState; // 0x18
    int button2QueryState; // 0x1C
    int button3QueryState; // 0x20
    int button0State; // 0x24
    int button1State; // 0x28
    int button2State; // 0x2C
    int button3State; // 0x30
    int deltaX; // 0x34
    int deltaY; // 0x38
    int wheel; // 0x3C
    float positionX; // 0x40
    float positionY; // 0x44
};

#define OFF(f, o) typedef char DirectInputMouseState_off_##f[((int)&((DirectInputMouseState*)0)->f == (o)) ? 1 : -1]
OFF(registry, 0x4);
OFF(inputVtable, 0x8);
OFF(enabled, 0xC);
OFF(changed, 0xD);
OFF(lastChangeTime, 0x10);
OFF(button0QueryState, 0x14);
OFF(button1QueryState, 0x18);
OFF(button2QueryState, 0x1C);
OFF(button3QueryState, 0x20);
OFF(button0State, 0x24);
OFF(button1State, 0x28);
OFF(button2State, 0x2C);
OFF(button3State, 0x30);
OFF(deltaX, 0x34);
OFF(deltaY, 0x38);
OFF(wheel, 0x3C);
OFF(positionX, 0x40);
OFF(positionY, 0x44);
#undef OFF
#define AsDirectInputMouseState(p) ((DirectInputMouseState*)(p))

#endif

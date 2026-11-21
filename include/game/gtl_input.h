#ifndef GAME_GTL_INPUT_H
#define GAME_GTL_INPUT_H

// gtl's input-device classes (InputEx), as their run-time type information names them. Layouts come from the
// constructors (0x803E8680-0x803E9C40) and the sizes InputEx_Create* allocate. Only the constructors are real C++
// here: they need the class hierarchy for their exception tables (a failed button `new` frees the memory with the
// class's operator delete, and a throwing constructor body destroys the CInputExDeviceBase base). The other member
// functions are still plain functions under their decomp names.

#include "types.h"

// reference count first, then the vtable; the deleting destructor is the first virtual
class IUnknownCustom {
public:
    s32 m_refCount;
    IUnknownCustom() : m_refCount(1) {
    }
    virtual ~IUnknownCustom();
};

namespace gtl {

// every input object is allocated from the input driver's allocator (g_inputAllocator)
class IInputResourceMemOperator {
public:
    static void* operator new(size_t size);
    static void operator delete(void* p);
};

class IInputExObject : public IUnknownCustom, public IInputResourceMemOperator {
public:
    virtual ~IInputExObject();
};

class IInputExButton : public IInputExObject {
public:
    IInputExButton();
    virtual ~IInputExButton();
    f32 m_threshold;
    s32 m_repeatInterval;
    s32 m_repeatDelay;
    u32 m_state;           // std::bitset<4>: down, pressed, repeat, released
    u8 m_wasDown;
    s32 m_repeatCounter;
    s32 m_heldFrames;
    char unk24[4];
};

class CInputExDigitalButton : public IInputExButton {
public:
    CInputExDigitalButton() {
    }
    virtual ~CInputExDigitalButton();
};

class CInputExAnalogButton : public IInputExButton {
public:
    CInputExAnalogButton() {
    }
    virtual ~CInputExAnalogButton();
};

class CInputExDeviceBase : public IUnknownCustom, public IInputResourceMemOperator {
public:
    CInputExDeviceBase() : m_device(0), m_active(0) {
    }
    virtual ~CInputExDeviceBase();
    void* m_device;        // the driver's IInputDevice (reference counted)
    u8 m_active;
    char unkD[7];
};

class CPad : public CInputExDeviceBase {
public:
    CPad();
    virtual ~CPad();
    char m_samples[0xB00];
    char m_history[0x180];
    IInputExButton* m_buttons[50];  // 20 digital, then 30 analog
};

class CKeyboard : public CInputExDeviceBase {
public:
    CKeyboard();
    virtual ~CKeyboard();
    u8 m_keys[0x100];
    IInputExButton* m_buttons[0x100];
};

struct InputVec2 {
    f32 x;
    f32 y;
    InputVec2() {}
    InputVec2(f32 x_, f32 y_) : x(x_), y(y_) {}
};

class CMouse : public CInputExDeviceBase {
public:
    CMouse();
    virtual ~CMouse();
    char m_sample[0x10];
    IInputExButton* m_buttons[6];  // 4 digital, then 2 analog (the wheel)
    InputVec2 m_position;
    InputVec2 m_screenSize;
};

// Cached KPAD filter request; SetSamplingParameter forwards both floats unchanged.
struct ScreenPointerSamplingParameter {
    f32 playRadius;
    f32 sensitivity;
};

class CScreenPointer : public CInputExDeviceBase {
public:
    CScreenPointer();
    virtual ~CScreenPointer();
    char m_samples[0xB00];
    char m_history[0x240];
    IInputExButton* m_buttons[16];
    ScreenPointerSamplingParameter m_samplingParameters[4];
    u8 m_flag;
};

} // namespace gtl

#endif

#ifndef GAME_GTL_INPUT_H
#define GAME_GTL_INPUT_H

// gtl's input-device classes (InputEx), as their run-time type information names them. Layouts come from the
// constructors (0x803E8680-0x803E9C40) and the sizes InputEx_Create* allocate. Only the constructors are real C++
// here: they need the class hierarchy for their exception tables (a failed button `new` frees the memory with the
// class's operator delete, and a throwing constructor body destroys the CInputExDeviceBase base). CInputExDeviceBase
// also has its member functions here; the rest are still plain functions under their decomp names.

#include "types.h"

// reference count first, then the vtable; the deleting destructor is the first virtual
class IUnknownCustom {
public:
    s32 m_refCount;
    IUnknownCustom() : m_refCount(1) {}
    virtual ~IUnknownCustom();
};

struct GtlInputExInitParams; // defined with Initialize in gtl_transform_frustum_stopwatch_input.cpp

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
    int GetSampleType();
    void* Destroy(int flags);
    s32 ProcessSample(void* sample);
    int vfunc_1();
    int vfunc_2();
    void SetThreshold(float threshold);
    void SetRepeatInterval(int interval);
    void SetRepeatDelay(int delay);
    int UpdateDigitalState(int pressed);
    int ResetDigitalHistory();
    f32 m_threshold;
    s32 m_repeatInterval;
    s32 m_repeatDelay;
    u32 m_state; // std::bitset<4>: down, pressed, repeat, released
    u8 m_wasDown;
    s32 m_repeatCounter;
    s32 m_heldFrames;
    union {
        u8 digitalDown;
        f32 analogValue;
    } m_sample; // 0x24: latched driver value
};

// Descriptor the input driver passes to a button latch: type 1 carries a float pointer.
struct DriverButtonSample {
    int type;    // +0: analog accepts exactly 1
    void* value; // +4: int* digital, float* analog
};

class CInputExDigitalButton : public IInputExButton {
public:
    CInputExDigitalButton() {}
    virtual ~CInputExDigitalButton();
    void* Destroy(int flags);
    int GetSampleType();
    int LatchDigitalDown(DriverButtonSample* arg);
    void ApplyLatchedSample();
    float GetLevel();
    int ResetSampleHistory();
};

class CInputExAnalogButton : public IInputExButton {
public:
    CInputExAnalogButton() {}
    void* Destroy(int flags);
    float GetAnalogValue();
    int LatchAnalogValue(DriverButtonSample* arg);
    void UpdateFromAnalog();
    int ResetState();
    int vfunc_4();
};

class CInputExDeviceBase : public IUnknownCustom, public IInputResourceMemOperator {
public:
    CInputExDeviceBase() : m_device(0), m_initialized(0) {}
    virtual ~CInputExDeviceBase();

    int Initialize(const GtlInputExInitParams* params);
    int Poll();
    s32 UpdateButtons(s32* buf, s32 count);
    int ReadItemValueOrRange(s32 valueKind, s32 itemKey, void* out);
    unsigned int GetButtonDown();
    int GetButtonPressed();
    s32 GetDeviceResult(s32* resultOut);
    s32 GetProviderId();

    void* m_device;   // 0x08: the driver's IInputDevice (reference counted)
    u8 m_initialized; // 0x0C
    char unkD[3];
    union {
        s32 m_channel;       // 0x10: Wii Remote KPAD channel; used by CScreenPointer only
        void* m_buttonTable; // 0x10: button configuration table, read by Initialize
    };
};

class CPad : public CInputExDeviceBase {
public:
    CPad();
    virtual ~CPad();
    void* Destroy(int flags);
    void* vfunc_1();
    const char* GetItemKey(s32 index);
    void* GetButton(s32 index);
    s32 UpdateLogicalButtonsFromSample();
    s32 ResetButtons();
    static s32 GetButtonCount();
    static s32 GetDriverStateSizeBytes();
    char m_samples[0xB00];
    char m_history[0x180];
    IInputExButton* m_buttons[50]; // 20 digital, then 30 analog
};

class CKeyboard : public CInputExDeviceBase {
public:
    CKeyboard();
    virtual ~CKeyboard();

    // Deleting teardown: releases the 256 buttons and the driver reference, frees self when flags > 0.
    void* Destroy(int flags);
    s32 GetItemKey(s32 index);
    IInputExButton* GetButton(s32 index);
    void AdaptKeySample();
    void* GetKeyBytes();
    s32 RunAllButtonCallbacks();
    static s32 GetButtonCount();
    static s32 GetDriverStateSizeBytes();

    u8 m_keys[0x100];                 // 0x14: 256 provider key bytes
    IInputExButton* m_buttons[0x100]; // 0x114: 256 digital buttons
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

    // Deleting teardown: releases the six buttons and the driver reference, frees self when flags > 0.
    void* Destroy(int flags);
    void* GetRawSample();
    void* GetItemKey(s32 index);
    void* GetButton(s32 index);
    s32 AdaptRelativeSample();
    s32 ResetButtons();
    static s32 GetButtonCount();
    static s32 GetDriverStateSizeBytes();

    char m_sample[0x10];          // 0x14: relative motion and button sample
    IInputExButton* m_buttons[6]; // 0x24: 4 digital, then 2 analog (the wheel)
    InputVec2 m_position;         // 0x3C
    InputVec2 m_screenSize;       // 0x44
};

// Cached KPAD filter request; SetSamplingParameter forwards both floats unchanged.
struct ScreenPointerSamplingParameter {
    f32 playRadius;  // +0 KPAD play radius, NOT a minimum sample value.
    f32 sensitivity; // +4 KPAD sensitivity, NOT a maximum sample value.
};

// One raw provider record (0xB0 bytes) as the input driver fills it. Unused bytes are not interpreted.
struct ScreenPointerRawSample {
    int parameter00, parameter04; // GetItemParameter(1,0/4)[1]
    char reserved08[4];
    int parameter0C, parameter10; // GetItemParameter(1,0xC/0x10)[1]
    char reserved14[0x1C];
    signed char buttons[16]; // +30: any nonzero byte means pressed
    char reserved40[0x10];
    int parameter50, parameter54, parameter58, parameter5C; // normalized likewise
    int nonnegativeResult;                                  // +60: negative values map to zero
    char reserved64[0x4C];
};
typedef char ScreenPointerRawSampleSize[(sizeof(ScreenPointerRawSample) == 0xB0) ? 1 : -1];

// One normalized history entry (0x24 bytes): GetSample copies it out; PollAndNormalizeHistory fills it.
struct ScreenPointerSample {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    f32 unk1C;
    s32 unk20;
};
typedef char ScreenPointerSampleSize[(sizeof(ScreenPointerSample) == 0x24) ? 1 : -1];

class CScreenPointer : public CInputExDeviceBase {
public:
    CScreenPointer();
    virtual ~CScreenPointer();

    // Deleting teardown: releases the 16 buttons and the driver reference, frees self when flags > 0.
    void* Destroy(int flags);

    void* GetRawSamples();
    s32 PollAndNormalizeHistory();
    s32 ResetButtons();

    void* GetButton(s32 index);
    void* GetItemKey(s32 index);

    s32 GetSample(s32 index, ScreenPointerSample* out) const;
    s32 GetSamplingParameter(u32 index, ScreenPointerSamplingParameter* out);
    s32 SetSamplingParameter(s32 index, ScreenPointerSamplingParameter* param);
    static s32 GetButtonCount();
    static s32 GetDriverStateSizeBytes();

    ScreenPointerRawSample m_samples[16];                   // 0x14: 16 raw provider records
    ScreenPointerSample m_history[16];                      // 0xB14: normalized history
    IInputExButton* m_buttons[16];                          // 0xD54
    ScreenPointerSamplingParameter m_samplingParameters[4]; // 0xD94
    u8 m_flag;                                              // 0xDB4
};

} // namespace gtl

#endif

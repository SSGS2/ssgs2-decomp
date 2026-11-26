// Wii Remote screen pointer: reset all logical button histories on the InputEx
// device's reset hook. API map: gtl::CScreenPointer::ResetButtons iterates its 16 buttons;
// constructor/allocation is in gtl_input_ex_devices_part1.cpp, shared class in game/gtl_input.h,
// digital-button reset in gtl_input_ex_button_part2.cpp. No pointer-coordinate or gameplay tuning.

#include "sdk_protos.h"
#include "game/gtl_input.h"

typedef void (gtl::IInputExButton::*ButtonMethod)();

// The game data holds a 12-byte CodeWarrior member-function record {this adjustment, slot, code token}.
// std::mem_fun(&IInputExButton::f) and std::for_each, as the game inlines them: the member-function pointer is
// copied once into the mem_fun object and once more into for_each's by-value functor parameter.
struct ButtonMemFun {
    ButtonMethod method;
    // Copy the ABI member-function descriptor, including its receiver adjustment.
    ButtonMemFun(ButtonMethod m) : method(m) {}
    // Invoke reset on one required live button; ignore its actual status result.
    void operator()(gtl::IInputExButton* b) const {
        (b->*method)();
    }
};

// Visit [first,last) in order, returning the by-value functor; it must not mutate
// the range or destroy buttons. Preserve both functor copies for the original ABI.
template <class F> static inline F ForEachButton(gtl::IInputExButton** first, gtl::IInputExButton** last, F f) {
    for (; first != last; ++first)
        f(*first);
    return f;
}

// Original member descriptor {this-adjust=0, vtable-byte-slot=0x14, vptr-offset=4}.
// Slot +0x14 of CInputExDigitalButton's table is the sample-history reset in gtl_input_ex_button_part2.cpp:
// clear state bits +14, previous-down +18, repeat counter +1C, held frames +20,
// sampled-down +24. Threshold/repeat configuration and pointer position survive.
// The void member-call view discards the implementation's return value of one.
extern ButtonMethod s_screenPointerButtonResetMethod;

namespace gtl {

// self is required, fully constructed, with 16 live buttons. Always returns 1;
// no allocation/ownership transfer. Serialize with sampling so reset histories
// are not overwritten mid-iteration.
s32 CScreenPointer::ResetButtons() {
    gtl::IInputExButton** last = m_buttons + 16;
    ForEachButton(m_buttons, last, ButtonMemFun(s_screenPointerButtonResetMethod));
    return 1;
}

} // namespace gtl

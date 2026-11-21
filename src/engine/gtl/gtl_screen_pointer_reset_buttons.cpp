// Wii Remote screen pointer: reset all logical button histories on the InputEx
// device's reset hook. API map: gtl_CScreenPointer_ResetButtons iterates its 16 buttons;
// constructor/allocation is in gtl_input_ex_devices_part1.cpp, shared class in game/gtl_input.h,
// digital-button reset in gtl_input_ex_button_part2.cpp. No pointer-coordinate or gameplay tuning.
// Translation unit 0x803E9BC0-0x803E9C40 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct gtl_IInputExButton;
// The game data holds a 12-byte CodeWarrior member-function record {this adjustment, slot, code token}.
typedef void (gtl_IInputExButton::*ButtonMethod)();

// Local 32-bit Wii view of CScreenPointer::m_buttons, not an owning container.
// Constructor creates 16 digital buttons with the input allocator; device
// destruction releases them. This reset only borrows them and retains no slots.
struct ScreenPointerButtonRange {
    char deviceAndHistory[0xD54];
    gtl_IInputExButton* buttons[16]; // [D54,D94), four-byte pointers, no null holes.
};

// std::mem_fun(&IInputExButton::f) and std::for_each, as the game inlines them: the member-function pointer is
// copied once into the mem_fun object and once more into for_each's by-value functor parameter.
struct ButtonMemFun {
    ButtonMethod method;
    // Copy the ABI member-function descriptor, including its receiver adjustment.
    ButtonMemFun(ButtonMethod m) : method(m) {}
    // Invoke reset on one required live button; ignore its actual status result.
    void operator()(gtl_IInputExButton* b) const { (b->*method)(); }
};

// Visit [first,last) in order, returning the by-value functor; it must not mutate
// the range or destroy buttons. Preserve both functor copies for the original ABI.
template <class F> static inline F ForEachButton(gtl_IInputExButton** first, gtl_IInputExButton** last, F f) {
    for (; first != last; ++first) f(*first);
    return f;
}

extern "C" {

// Original member descriptor {this-adjust=0, vtable-byte-slot=0x14, vptr-offset=4}.
// Slot +0x14 of CInputExDigitalButton's table resolves to vfunc_3 (gtl_input_ex_button_part2.cpp):
// clear state bits +14, previous-down +18, repeat counter +1C, held frames +20,
// sampled-down +24. Threshold/repeat configuration and pointer position survive.
// The void member-call view discards the implementation's return value of one.
extern ButtonMethod s_screenPointerButtonResetMethod;

// self is required, fully constructed, with 16 live buttons. Always returns 1;
// no allocation/ownership transfer. Serialize with sampling so reset histories
// are not overwritten mid-iteration. Raw byte endpoints preserve compiled ABI;
// ScreenPointerButtonRange gives their typed extent and lifetime above.
int gtl_CScreenPointer_ResetButtons(char* self) {
    gtl_IInputExButton** last = (gtl_IInputExButton**)(self + 0xd94);
    ForEachButton((gtl_IInputExButton**)(self + 0xd54), last, ButtonMemFun(s_screenPointerButtonResetMethod));
    return 1;
}

} // extern "C"

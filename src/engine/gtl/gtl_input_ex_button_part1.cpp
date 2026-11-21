// GTL controller/keyboard/mouse button state and auto-repeat (InputEx).
// Mod index: constructor defaults and SetRepeatDelay/SetRepeatInterval set timing;
// UpdateDigitalState produces down/press/repeat/release bits, consumed by InputEx
// queries. SetThreshold controls the analog comparison in gtl_input_ex_button_part2.cpp, not this
// digital update. Layout/ownership: game/gtl_input.h, driver allocator teardown
// below. No controller mapping or hardware polling occurs here.
// Translation unit 0x803EAC48-0x803EB544 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/stdexcept_local.h"
#include "sdk_protos.h"
#include "game/gtl_input.h"

extern "C" {

extern char s_bitsetTestOutOfRangeMessage[];
extern char s_bitsetSetOutOfRangeMessage[];
extern char s_outOfRangeTypeNames[];
extern char __vt__Q23gtl14IInputExButton[];
extern const float kInputExButtonDefaultThreshold;

}

// Use the constructor's shared schema instead of a second offset-only view:
// +00 refcount, +04 vptr, +08 threshold, +0C interval, +10 delay,
// +14 state word, +18 previous-down byte, +1C countdown, +20 held-call count.
typedef gtl::IInputExButton IInputExButton;
enum ButtonStateBit { kButtonDown = 0, kButtonPressed = 1,
    kButtonRepeat = 2, kButtonReleased = 3 };

struct BitRef4 {
    unsigned int* words;
    unsigned int index;
    // Borrow a state word; index must be one of the four ButtonStateBit values.
    BitRef4(unsigned int* p, unsigned int i) {
        words = p;
        index = i;
    }
    // Read a single bit, retaining the original out-of-range exception path.
    int test() const {
        unsigned int i = index;
        if (i >= 4) {
            std::out_of_range error(s_bitsetTestOutOfRangeMessage, s_outOfRangeTypeNames);
        }
        return (*words & (1u << i)) != 0;
    }
    // Change one bit only; every nonzero value sets it.
    void set(int value) const {
        unsigned int i = index;
        if (i >= 4) {
            std::out_of_range error(s_bitsetSetOutOfRangeMessage, s_outOfRangeTypeNames);
        }
        unsigned int mask = 1u << i;
        if (value != 0) {
            *words |= mask;
        } else {
            *words &= ~mask;
        }
    }
};

// Construct a borrowed proxy; nested factories preserve exception inline depth.
inline BitRef4 makeBitRef4(unsigned int* p, unsigned int i) {
    BitRef4 view(p, i);
    return view;
}
// Keep the reference factory's inline depth aligned with the exception guards.
inline BitRef4 bitRef4Unchecked(unsigned int* p, unsigned int i) {
    return makeBitRef4(p, i);
}
// Public local proxy factory; range checks occur on read/write, not construction.
inline BitRef4 bitRef4(unsigned int* p, unsigned int i) {
    return bitRef4Unchecked(p, i);
}

extern "C" {

#pragma push
#pragma use_lmw_stmw on
// Base sample-admission hook: unsupported (0); receiver is unused, unchanged.
int gtl_IInputExButton_vfunc_1(IInputExButton* self) {
    return 0;
}

// Base poll/update hook: unsupported (0); receiver is unused, unchanged.
int gtl_IInputExButton_vfunc_2(IInputExButton* self) {
    return 0;
}

} // extern "C"

// Start released, with no held calls or pending repeat. Interval=2, delay=15
// count UpdateDigitalState calls, not milliseconds or video frames. Repetition
// occurs on countdown < 0: delay+1 calls after the initial press, then interval+1
// calls between pulses (16 and 3 at defaults). threshold is in the sampled analog
// value's units: gtl_input_ex_button_part2.cpp compares sample >= threshold; no normalization here.
// kInputExButtonDefaultThreshold supplies default 0.7f (stored 0.699999988079071); changing it
// shifts analog activation, not digital-button behavior. Useful bounds depend on
// the device's sample range; no universal safe range is established here.
gtl::IInputExButton::IInputExButton() : m_state(0), m_wasDown(0), m_repeatCounter(0), m_heldFrames(0) {
    Memory_Clear(&m_threshold, 0xc);
    Memory_Clear(&m_state, 0x4);
    m_threshold = kInputExButtonDefaultThreshold;
    m_repeatInterval = 2;
    m_repeatDelay = 0xf;
}

extern "C" {

void __dl__Q23gtl25IInputResourceMemOperatorFPv(void* self);

// gtl::IInputExObject::~IInputExObject(): nothing to destroy but the memory (kept as a plain function: a real
// destructor would store the class's vtable first, which the game's does not)
void* __dt__Q23gtl14IInputExObjectFv(void* self, int count) {
    if (self != 0 && count > 0) {
        __dl__Q23gtl25IInputResourceMemOperatorFPv(self);
    }
    return self;
}

// Store an analog activation threshold unchanged; required live receiver.
// No validation/clamp: a useful threshold depends on the device sample range.
void gtl_IInputExButton_SetThreshold(IInputExButton* self, float threshold) {
    self->m_threshold = threshold;
}

// Store calls-between-repeat countdown reset; nonnegative values avoid signed
// underflow. Does not reset an already running countdown. No validation occurs.
void gtl_IInputExButton_SetRepeatInterval(IInputExButton* self, int interval) {
    self->m_repeatInterval = interval;
}

// Store initial repeat countdown, in update calls; same constraints as interval.
// Takes effect on the next rising edge, without altering current state/counters.
void gtl_IInputExButton_SetRepeatDelay(IInputExButton* self, int delay) {
    self->m_repeatDelay = delay;
}

// Required writable receiver, canonical pressed=0 or 1 (callers normalize in
// gtl_input_ex_button_part2.cpp); arbitrary nonzero integers break comparison with previous byte.
// Always returns status 1; data is in m_state bits 0 down, 1 rising edge,
// 2 rising edge OR repeat pulse, 3 falling edge. Unrelated state bits survive.
// Updates previous-down, saturating held-call count, and repeat countdown;
// release clears both counters. Poll once per intended input tick, serialized
// with readers/setters. No allocation, retention or normal failure path.
int gtl_IInputExButton_UpdateDigitalState(IInputExButton* self, int pressed) {
    self->m_wasDown = bitRef4(&self->m_state, kButtonDown).test();
    bitRef4(&self->m_state, kButtonDown).set(pressed);
    if (pressed != 0) {
        if (self->m_heldFrames < 0x7FFFFFFF) {
            self->m_heldFrames = self->m_heldFrames + 1;
        }
    } else {
        self->m_heldFrames = 0;
    }
    int rising = 0;
    if ((unsigned int)pressed != self->m_wasDown && pressed != 0) {
        rising = 1;
    }
    bitRef4(&self->m_state, kButtonPressed).set(rising);
    int falling = 0;
    if ((unsigned int)pressed != self->m_wasDown && pressed == 0) {
        falling = 1;
    }
    bitRef4(&self->m_state, kButtonReleased).set(falling);
    int repeatPulse = bitRef4(&self->m_state, kButtonPressed).test();
    if (pressed != 0) {
        self->m_repeatCounter = self->m_repeatCounter - 1;
        if (bitRef4(&self->m_state, kButtonPressed).test()) {
            self->m_repeatCounter = self->m_repeatDelay;
        } else if (self->m_repeatCounter < 0) {
            int reset = self->m_repeatInterval;
            repeatPulse = !repeatPulse;
            self->m_repeatCounter = reset;
        }
    } else {
        self->m_repeatCounter = 0;
    }
    bitRef4(&self->m_state, kButtonRepeat).set(repeatPulse);
    return 1;
}



#pragma pop

} // extern "C"

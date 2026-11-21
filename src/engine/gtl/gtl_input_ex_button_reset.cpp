// InputEx controller button reset. Configuration and digital transition/repeat
// logic live in gtl_input_ex_button_part1.cpp; the canonical class is game/gtl_input.h.
// Mod map: threshold/repeat interval/repeat delay are configuration, preserved by
// this reset. Button transitions and repeat cadence are in UpdateDigitalState.
// Translation unit 0x803EB544-0x803EB580 (1 function).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// Local ABI view of gtl::IInputExButton (no virtuals/RTTI emitted). Offsets match
// m_state/m_wasDown/m_repeatCounter/m_heldFrames in game/gtl_input.h.
struct ButtonResetView {
    char reserved00[0x14];
    unsigned int stateBits;     // +14: bits 0 down, 1 pressed, 2 repeat, 3 released
    unsigned char wasDown;      // +18: previous digital sample, 0 or 1
    int repeatCounter;         // +1C: signed countdown in digital update calls
    int heldUpdates;           // +20: consecutive down calls, saturates at INT_MAX
};
typedef char ButtonResetView_size[(sizeof(ButtonResetView) == 0x24) ? 1 : -1];

extern "C" {

// Reset sampled/edge/repeat history to released without changing configuration.
// Required writable self is borrowed; no allocation, callback or failure path.
// Returns 1 (success), not button-down state. The next pressed sample creates a
// fresh rising edge and seeds the repeat delay in UpdateDigitalState. Reset must
// be serialized with sampling/readers; there is no lock in this operation.
int gtl_IInputExButton_ResetDigitalHistory(ButtonResetView* self) {
    self->wasDown = 0;
    self->repeatCounter = 0;
    self->heldUpdates = 0;
    Memory_Clear(&self->stateBits, 4);
    return 1;
}

} // extern "C"

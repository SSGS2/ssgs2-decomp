// InputEx controller button reset. Configuration and digital transition/repeat
// logic live in gtl_input_ex_button_part1.cpp; the canonical class is game/gtl_input.h.
// Mod map: threshold/repeat interval/repeat delay are configuration, preserved by
// this reset. Button transitions and repeat cadence are in UpdateDigitalState.

#include "sdk_protos.h"
#include "game/gtl_input.h"

// Reset sampled/edge/repeat history to released without changing configuration.
// Required writable self is borrowed; no allocation, callback or failure path.
// Returns 1 (success), not button-down state. The next pressed sample creates a
// fresh rising edge and seeds the repeat delay in UpdateDigitalState. Reset must
// be serialized with sampling/readers; there is no lock in this operation.
int gtl::IInputExButton::ResetDigitalHistory() {
    m_wasDown = 0;
    m_repeatCounter = 0;
    m_heldFrames = 0;
    Memory_Clear(&m_state, 4);
    return 1;
}

// gtl::CInputExAnalogButton, the analog variant of an extended controller button. This file holds
// only its deleting destructor; the latch and update members are in gtl_input_ex_button_part2.cpp.
#include "types.h"
#include "game/gtl_input.h"

// Deleting destructor of gtl::CInputExAnalogButton: frees the object when flags is positive and returns it.
void* gtl::CInputExAnalogButton::Destroy(int flags) {
    if (this != 0 && flags > 0) {
        IInputResourceMemOperator::operator delete(this);
    }
    return this;
}

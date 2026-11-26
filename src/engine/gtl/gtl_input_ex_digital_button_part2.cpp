// gtl::CInputExDigitalButton deleting destructor: frees the object when flags is positive and returns it.
#include "types.h"
#include "game/gtl_input.h"

void* gtl::CInputExDigitalButton::Destroy(int flags) {
    if (this != 0 && flags > 0) {
        IInputResourceMemOperator::operator delete(this);
    }
    return this;
}

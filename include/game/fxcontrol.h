#pragma once
// Root of the CFx effect-control hierarchy (RTTI of CFxRunDust etc. lists __RTTI__10CFxControl as base).
// The destructor is out of line (previously the placeholder __dt__10CFxControlFv, defined in cfx_shot_effects.cpp).
struct CFxControl {
    void* vtable;
    int result;
    ~CFxControl();
};

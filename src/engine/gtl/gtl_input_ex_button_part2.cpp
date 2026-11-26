// Extended controller buttons: latch driver samples, then derive press/release
// and repeat events once per input update. API/settings map: SetThreshold,
// SetRepeatInterval, SetRepeatDelay and UpdateDigitalState in gtl_input_ex_button_part1.cpp;
// canonical base layout/defaults in game/gtl_input.h. No physical threshold unit
// is imposed: compare the driver's sample and threshold in the SAME input scale.
// Repeat counters count UpdateDigitalState calls, not elapsed milliseconds.
// All receivers must be live button objects; serialize these hooks on
// the input-update thread. No hook retains a sample pointer or acquires a lock.

#include "sdk_protos.h"
#include "game/gtl_input.h"

using gtl::DriverButtonSample;

extern "C" {

extern float kInputExButtonDownLevel; // 1.0f: digital pressed output level
extern float kInputExButtonZeroLevel; // 0.0f: digital released level and analog reset sample

} // extern "C"

// Latch required descriptor/value as Boolean. Digital ignores descriptor type;
// unlike analog it performs no null checks. Returns 1; history changes in slot 2.
int gtl::CInputExDigitalButton::LatchDigitalDown(DriverButtonSample* driverSample) {
    int driverValue = *(int*)driverSample->value;
    m_sample.digitalDown = driverValue != 0;
    return 1;
}

// Consume the latched sample; update state bits, previous-down and both counters.
void gtl::CInputExDigitalButton::ApplyLatchedSample() {
    UpdateDigitalState(m_sample.digitalDown);
}

// Return the configured pressed/released float level for the current latch.
float gtl::CInputExDigitalButton::GetLevel() {
    if (m_sample.digitalDown) {
        return kInputExButtonDownLevel;
    }
    return kInputExButtonZeroLevel;
}

// Reset sample/history, returning 1. Preserve threshold and repeat settings;
// Memory_Clear covers only the four-byte event bitset, not the whole object.
int gtl::CInputExDigitalButton::ResetSampleHistory() {
    m_wasDown = 0;
    m_repeatCounter = 0;
    m_heldFrames = 0;
    Memory_Clear(&m_state, 4);
    m_sample.digitalDown = 0;
    return 1;
}

// Return the latched analog sample verbatim; no normalization/clamping here.
float gtl::CInputExAnalogButton::GetAnalogValue() {
    return m_sample.analogValue;
}

// Latch one float from a type-1 descriptor; return 0 for null/wrong type/null
// value, leaving the old sample and event history unchanged. Otherwise return 1.
int gtl::CInputExAnalogButton::LatchAnalogValue(DriverButtonSample* driverSample) {
    if (driverSample == 0) {
        return 0;
    }
    if (driverSample->type != 1) {
        return 0;
    }
    float* driverValue = (float*)driverSample->value;
    if (driverValue == 0) {
        return 0;
    }
    m_sample.analogValue = *driverValue;
    return 1;
}

// Inclusive threshold creates the down bit consumed by the same edge/repeat
// logic as digital input. NaN compares false. Threshold changes affect the next
// update; they do not modify the cached float returned by GetAnalogValue.
void gtl::CInputExAnalogButton::UpdateFromAnalog() {
    ((gtl::IInputExButton*)this)->UpdateDigitalState(m_sample.analogValue >= m_threshold);
}

// Reset the analog latch to the released level and clear all event history;
// leave threshold/repeat tuning intact. Return 1; no allocation or device I/O.
int gtl::CInputExAnalogButton::ResetState() {
    m_wasDown = 0;
    m_repeatCounter = 0;
    m_heldFrames = 0;
    Memory_Clear(&m_state, 4);
    m_sample.analogValue = kInputExButtonZeroLevel;
    return 1;
}

// Successful no-op hook; receiver is unused, cached sample/history untouched.
int gtl::CInputExAnalogButton::vfunc_4() {
    return 1;
}

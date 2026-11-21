// Extended controller buttons: latch driver samples, then derive press/release
// and repeat events once per input update. API/settings map: SetThreshold,
// SetRepeatInterval, SetRepeatDelay and UpdateDigitalState in gtl_input_ex_button_part1.cpp;
// canonical base layout/defaults in game/gtl_input.h. No physical threshold unit
// is imposed: compare the driver's sample and threshold in the SAME input scale.
// Repeat counters count UpdateDigitalState calls, not elapsed milliseconds.
// All receivers must be live ButtonSampleView objects; serialize these hooks on
// the input-update thread. No hook retains a sample pointer or acquires a lock.
// Translation unit 0x803EB580-0x803EB6DC (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// Wii ABI overlay of gtl::IInputExButton and its four-byte sample tail. Samples
// are copied, never retained: caller may release the descriptor after vfunc_1.
struct ButtonSampleView {
    int referenceCount;                  // +00
    void* vtable;                        // +04
    float threshold;                     // +08
    int repeatInterval;                  // +0C, update calls
    int repeatDelay;                     // +10, update calls
    unsigned int stateBits;              // +14: down/pressed/repeat/released
    unsigned char wasDown;               // +18: previous update's down bit
    char padding19[3];
    int repeatCountdown;                 // +1C
    int heldUpdateCount;                 // +20, saturates at INT_MAX
    union { unsigned char digitalDown; float analogValue; } sample; // +24
};
struct DriverButtonSample {
    int type;                            // +0: analog accepts exactly 1
    void* value;                         // +4: int* digital, float* analog
};

extern "C" {

void gtl_IInputExButton_UpdateDigitalState(void* self, int state);

extern float kInputExButtonDownLevel; // 1.0f: digital pressed output level
extern float kInputExButtonZeroLevel; // 0.0f: digital released level and analog reset sample

// Latch required descriptor/value as Boolean. Digital ignores descriptor type;
// unlike analog it performs no null checks. Returns 1; history changes in slot 2.
int gtl_CInputExDigitalButton_LatchDigitalDown(void* self, void* arg) {
    int driverValue = *(int*)((DriverButtonSample*)arg)->value;
    ((ButtonSampleView*)self)->sample.digitalDown = driverValue != 0;
    return 1;
}

// Consume the latched sample; update state bits, previous-down and both counters.
void gtl_CInputExDigitalButton_ApplyLatchedSample(void* self) {
    gtl_IInputExButton_UpdateDigitalState(self, ((ButtonSampleView*)self)->sample.digitalDown);
}

// Return the configured pressed/released float level for the current latch.
float gtl_CInputExDigitalButton_GetLevel(void* self) {
    if (((ButtonSampleView*)self)->sample.digitalDown) {
        return kInputExButtonDownLevel;
    }
    return kInputExButtonZeroLevel;
}

// Reset sample/history, returning 1. Preserve threshold and repeat settings;
// Memory_Clear covers only the four-byte event bitset, not the whole object.
int gtl_CInputExDigitalButton_ResetSampleHistory(void* self) {
    ((ButtonSampleView*)self)->wasDown = 0;
    ((ButtonSampleView*)self)->repeatCountdown = 0;
    ((ButtonSampleView*)self)->heldUpdateCount = 0;
    Memory_Clear(&((ButtonSampleView*)self)->stateBits, 4);
    ((ButtonSampleView*)self)->sample.digitalDown = 0;
    return 1;
}

// Return the latched analog sample verbatim; no normalization/clamping here.
float gtl_CInputExAnalogButton_vfunc_5(void* self) {
    return ((ButtonSampleView*)self)->sample.analogValue;
}

// Latch one float from a type-1 descriptor; return 0 for null/wrong type/null
// value, leaving the old sample and event history unchanged. Otherwise return 1.
int gtl_CInputExAnalogButton_LatchAnalogValue(void* self, void* arg) {
    if (arg == 0) {
        return 0;
    }
    if (((DriverButtonSample*)arg)->type != 1) {
        return 0;
    }
    float* driverValue = (float*)((DriverButtonSample*)arg)->value;
    if (driverValue == 0) {
        return 0;
    }
    ((ButtonSampleView*)self)->sample.analogValue = *driverValue;
    return 1;
}

// Inclusive threshold creates the down bit consumed by the same edge/repeat
// logic as digital input. NaN compares false. Threshold changes affect the next
// update; they do not modify the cached float returned by slot 5.
void gtl_CInputExAnalogButton_UpdateFromAnalog(void* self) {
    gtl_IInputExButton_UpdateDigitalState(self, ((ButtonSampleView*)self)->sample.analogValue >= ((ButtonSampleView*)self)->threshold);
}

// Reset the analog latch to the released level and clear all event history;
// leave threshold/repeat tuning intact. Return 1; no allocation or device I/O.
int gtl_CInputExAnalogButton_ResetState(void* self) {
    ((ButtonSampleView*)self)->wasDown = 0;
    ((ButtonSampleView*)self)->repeatCountdown = 0;
    ((ButtonSampleView*)self)->heldUpdateCount = 0;
    Memory_Clear(&((ButtonSampleView*)self)->stateBits, 4);
    ((ButtonSampleView*)self)->sample.analogValue = kInputExButtonZeroLevel;
    return 1;
}

// Successful no-op hook; receiver is unused, cached sample/history untouched.
int gtl_CInputExAnalogButton_vfunc_4(void* self) {
    return 1;
}

} // extern "C"

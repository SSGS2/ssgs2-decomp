// Translation unit 0x80420CB0-0x80420CD0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

// Return the hardware time-base frequency in Hz as two high/low 32-bit words.
// frequencyWords must provide two writable words; returns 1 unconditionally.
// The OS low-memory word is the bus clock in Hz; the time base runs at one quarter.
int Timer_GetTickFrequency(unsigned int *frequencyWords) {
    enum { kBusClockFrequencyAddress = 0x800000F8, kBusClockToTimeBaseShift = 2 };
    unsigned int timerFrequencyHz = *(unsigned int *)kBusClockFrequencyAddress >> kBusClockToTimeBaseShift;
    frequencyWords[0] = 0;
    frequencyWords[1] = timerFrequencyHz;
    return 1;
}

} // extern "C"

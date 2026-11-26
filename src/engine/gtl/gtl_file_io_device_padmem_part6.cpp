// PADMEM file device: two virtual slots (slot 24 and slot 9) that return fixed values.
// They read no device state and change no ownership.

#include "classes/GtlFileIoDevicePADMEM.h"

// PADMEM virtual slots are indexed by phase1_class_tables.cpp; keep their ABI names.
enum PadMemDeviceConstants {
    kPadMemSlot24Result = 0x20,
    kPadMemSlot9Result = 1
};

// Return the fixed slot-24 value (32); its PADMEM-specific interpretation is unresolved.
// No arguments, object access or ownership changes. DVD slot 24 reports alignment,
// but there is no direct PADMEM call site here to establish the same contract.
int gtl::CFileIoDevicePADMEM::vfunc_24() {
    return kPadMemSlot24Result;
}

// Return the fixed slot-9 value (1); the meaning of this status is unresolved.
// No arguments, object access or ownership changes; do not infer a handle check.
int gtl::CFileIoDevicePADMEM::vfunc_9() {
    return kPadMemSlot9Result;
}
